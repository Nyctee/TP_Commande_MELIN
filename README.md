# TP systèmes d'acquisition et de commande
TP MSC 2022 MÉLIN Julien 

# Commande basique de la MCC
## Génération de 4 PWM
On souhaite générer à l'aide du Timer1 4 PMW en complémentaires décalées pour contrôler en boucle ouverte le moteur.
Pour se faire elles devront respecter les caractéristiques suivantes:

- une fréquence de 16kHz

- un temps mort d'au moins 2us

- résolution minimum de 10bits

On utilise alors la configuration suivante pour le Timer1
![Confiig_tim1](https://user-images.githubusercontent.com/46086052/210862710-4ef70d75-28a2-4dd0-aa01-8efedabad756.jpg)

On retrouve bien les 4PMW en sortie (CH1/CH1N et CH2/CH2N) et avec les valeurs choisies du PSC et de l'ARR 
on obtient bien les 16kHz à partir de l'horloge à 170Mhz. Pour la valeur du DeadTime il faut remplir le registre correspondant.
Avec une valeur de 210 on a bien un DeadTime supérieur à 2us.

# Prise en main du hacheur 

Pour piloter le moteur il nous faudra dans un premier temps câbler les PWM comme suit:

- commande YTop(CH1) sur le pin 12 et Ybot(CHI1N) sur le pin 30 du hacheur 

- commande RTop(CH2) sur le pin 13 et RBot(CH2N) sur le pin 31.

Il faudra aussi fournir le signal ISO_RESET sur le pin 33 du hacheur pour autoriser l'allumage du hacheur.
Ce signal consiste à une mise à 0 de la ligne pendant au moins 2us pour un retour à 1. 

# Les différentes commandes et les premiers tests 

## La commande start 


La commande de start doit permettre de lancer la séquence d'amorçage du hacheur. Elle se fait par réception de la commande start via la liasion uart grace au code suivant:

```
else if(strcmp(argv[0],"start")==0){

				HAL_GPIO_WritePin(ISO_RESET_GPIO_Port, ISO_RESET_Pin, SET);
				HAL_Delay(100);
				HAL_GPIO_WritePin(ISO_RESET_GPIO_Port, ISO_RESET_Pin, RESET);
				sprintf(uartTxBuffer,"ready\r\n");
				HAL_UART_Transmit(&huart2, uartTxBuffer, strlen(uartTxBuffer), HAL_MAX_DELAY);
			}
```

On utilise le shell fourni pour repérer l'entrée du mot "start" puis on lance la séquence d'amorçage et enfin on affiche à l'utilisateur que l'on est ready.

Le démarrage via l'appui sur le bouton bleu n'est pas implémenté car ce dernier va servir à autre chose plus tard. 

## Tests de vitesses et la commande speed

Les tests aux différents rapports cycliques demandés permettent d'effectuer les observations suivantes:

- Le rapport cyclique de 50% ne fait pas tourner le moteur, il correspond en fait au centre des vitesses possibles pour le moteur. Toutes valeurs supérieures feront tourner le moteur dans un sens alors que toutes valeurs inférieures le feront tourner dans l'autre sens.
- Un changement trop brutal dans la valeur de la vitesse entrainera un trop grand appel de courant qui mettra alors le hacheur en sécurité et stoppera le moteur, il faudra alors relancer la commande start.

Une fois ces différentes observations réalisées on peut s'attaquer à la commande speed qui nous permettra de contrôler le moteur.
```
else if(strcmp(argv[0],"speed=")==0){
				speed=atoi(argv[1]);
				if (speed<0){speed=0;}
				if (speed>100){speed=100;}

				j=(MAX_ARR*speed)/100;
				TIM1->CCR1=j;
				TIM1->CCR2=(MAX_ARR-j);
				sprintf(uartTxBuffer,"speed= : %d\r\n",atoi(argv[1]));
				HAL_UART_Transmit(&huart2, uartTxBuffer, strlen(uartTxBuffer), HAL_MAX_DELAY);

			}
```

On utilise à nouveau le shell pour détecter le mot "speed=" puis on récupère la valeur qui suit. On limite les valeurs possibles entre 0 et 100 pour éviter certains problèmes puis on effectue le calcul permettant de fournir la valeur correspondant au PWM. On affiche ensuite la valeur à l'utilisateur. 

## Commande stop et pinout 

Il existe deux commandes supplémentaires, la commande pinout affiche simplement tous les pins utilisés ainsi que leurs utilités. La commande stop elle force simplement le rapport cyclique à 50% pour arrêter le moteur. 

# Capteur de courant et position.

## Mesure de courant

Si on observe les pins 16 et 35 du connecteur à l'oscilloscope on obtient les résultats suivants: 
![courant_speed_80](https://user-images.githubusercontent.com/46086052/211040627-38cdfa28-d80a-46c3-bf9f-97db2c8b80da.jpg)
On a donc deux tenions qui correspondent au courant dans la phase jaune et celui dans la phase rouge. On observe qu'ils varient autour d'une valeur d'environ 2,5V.

Si on se réfère maintenant à la documentation du hacheur on peut trouver ce tableau. 
![tableau_courant](https://user-images.githubusercontent.com/46086052/211041999-4eeb65c5-78e7-483f-8769-e10329052359.jpg)
Pour obtenir la valeur du courant il faudra donc soustraire 2.5V et puis multiplié par 12. 

Pour se faire on utilisera le Timer1 pour effectuer une mesure toutes les secondes grâce à l'ADC1 et aux DMA. On utilise alors le code suivant: 

```
if(adcDmaFlag &&  affichage==1 && it2_Tim1){
			retour_courant=(((((float)adcBuffer[0])*(3.3/4095))-2.25)*12);
			sprintf(uartTxBuffer,"ADC Value1: %1.2f\r\n",retour_courant);
			HAL_UART_Transmit(&huart2, uartTxBuffer, strlen(uartTxBuffer), HAL_MAX_DELAY);
			adcDmaFlag=0;
			it2_Tim1=0;
		}
```

La mesure s'effectue donc quand on a 
- adcDmaFlag à '1', c'est-à-dire que l'ADC a effectué les mesures et les à stocker dans un tableau grace aux DMA
- affichage à '1', c'est la nouvelle fonction du bouton bleu, on peut contrôler l'affichage ou non des différentes mesures en appuyant dessus.
- it2_Tim1 à '1', c'est l'interruption engendrée par le Timer 1 toutes les secondes.

Quand ces trois conditions sont réunies on récupère la valeur stockée qu'on convertit en tension puis on applique la formule vue précédemment (on soustrait uniquement 2,25V car après plusieurs tests il s'avère que la valeur pour 0A n'est pas 2,5V mais bien 2,25V). 

## Mesure de la vitesse 

On cherche maintenant à récupérer la valeur de la vitesse à partir de l'encodeur. 
Pour se faire on va configurer un timer en mode encodeur et déduire de l'écart entre deux positions fournit par l'encodeur la vitesse du moteur. On utilisera donc comme précédemment le Timer1 pour effectuer une mesure toutes les secondes. 

Il reste alors à trouver le lien entre la vitesse et la valeur du timer en mode encodeur, pour cela on relève quelques valeurs et avec un simple produit en croix on obtient:
![tableau_vitesse](https://user-images.githubusercontent.com/46086052/211049802-e0ae7677-3d0c-43d3-8b65-3f9edf1539f7.jpg)

On a donc choisi de multiplier la valeur de l'encode par 0.03 pour retrouver la vitesse correspondante. Ce n'est pas ultra-précis mais cela reste fiable. 


On se retrouve donc avec le code suivant: 

```
if(it_Tim1 &&  affichage==2){
		position=TIM2->CNT;
		position= position-ENCOD_MOITIE;
		TIM2->CNT=ENCOD_MOITIE;
		sprintf(uartTxBuffer,"vitesse : %.1f tr/min\r\n",((float)position*0.03));
		HAL_UART_Transmit(&huart2, uartTxBuffer, strlen(uartTxBuffer), HAL_MAX_DELAY);
		it_Tim1=0;
		}
```

On a en conditions:
- it_Tim1 qui est généré toutes les secondes par le Timer 1.
- affichage==2 qui est encore une fois contrôlé par l'appui sur le bouton bleu.

On récupère alors la position grâce au Timer2 puis on la compare à la moitié de la valeur possible. On en déduit ensuite la vitesse avec le calcul vu précédemment. On n'oublie pas de remettre la valeur du compteur du Timer2 à la moitié pour éviter d'atteindre la valeur max. 
Il ne reste plus qu'à afficher la valeur et remettre le flag du Timer1 à 0. 

# Asservissement 

Il faut maintenant implémenter un asservissement du moteur en utilisant la méthode vue en TD mais je n'ai malheureusement pas eu le temps de le terminer.
