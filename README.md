# TP systèmes d'acquisition et de commande
TP MSC 2022 MÉLIN Julien 

# Commande basique de la MCC
## Génération de 4 PWM
On souhaite générer à l'aide du Timer1 4 PMW en complémentaires décalée pour contrôler en boucle ouverte le moteur.
Pour se faire elles devront respecter les caractéristiques suivantes:

- Une fréquence de 16khz

- Un temps mort d'au moins 2us

- Résolution minimum de 10bits

On utilise alors la configuration suivante pour le Timer1
![Confiig_tim1](https://user-images.githubusercontent.com/46086052/210862710-4ef70d75-28a2-4dd0-aa01-8efedabad756.jpg)

On retrouve bien les 4PMW en sortie (CH1/CH1N et CH2/CH2N) et avec les valeurs choisies du PSC et de l'ARR 
on obtient bien les 16kHz à partir de l'horloge à 170Mhz. Pour la valeur du DeadTime il faut remplir le registre correspondant.
Avec une valeur de 210 on à bien un DeadTime supérieur à 2us.

# Prise en main du hacheur 

Pour piloter le moteur il nous faudrat dans un premier temps cabler les PWM comme suit:

- Commande YTop(CH1) sur le pin 12 et Ybot(CHI1N) sur le pin 30 du hacheur 

- Commande RTop(CH2) sur le pin 13 et RBot(CH2N) sur le pin 31.

Il faudra aussi fournir le signal ISO_RESET sur le pin 33 du hacheur pour autoriser l'allumage du hacheur.
Ce signal consiste à une mise à 0 de la ligne pendant au moins 2us pour un retour à 1. 

# Les différentes commandes et les premier tests 

## La commande start 


La commande de start doit permettre de lancer la séquence d'amorçage du hacheur. Elle se fait pa réception de la commande start via la liasion uart grace au code suivant:

```
else if(strcmp(argv[0],"start")==0){

				HAL_GPIO_WritePin(ISO_RESET_GPIO_Port, ISO_RESET_Pin, SET);
				HAL_Delay(100);
				HAL_GPIO_WritePin(ISO_RESET_GPIO_Port, ISO_RESET_Pin, RESET);
				sprintf(uartTxBuffer,"ready\r\n");
				HAL_UART_Transmit(&huart2, uartTxBuffer, strlen(uartTxBuffer), HAL_MAX_DELAY);
			}
```

On utilise le shell fournit pour repérer l'entrer du mot "start" puis on lance la séquence d'amorçage et enfin on affiche à l'utilisateur que l'on est ready.

Le démarrage via l'appui sur le bouton bleu n'est pas implémenté car ce dernier va servir à autre chose plus tard. 

## Tests de vitesses et la commande speed

Les tests aux différents rapports cycliques demandés permettent d'éffectuer les observations suivantes:

- Le rapport cyclique de 50% ne fait pas tourner le moteur, il correspond en faite au centre des vitesses possibles pour le moteur. Toutes valeurs supérieures fera tourner le moteur dans un sens alors que toutes valeurs inférieur le fera tourner dans l'autre sens.
- Un changement trop brutal dans la valeur de la vitesse entrainera un trop grand appel de courant qui mettra alors le hacheur en sécurité et stopera le moteur, il faurdra alors relancer la commande start.

Une fois ces différentes observations réaliser on peut s'attaquer à la commande speed qui nosu permettra de controler le moteur.
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

On utilise à nouveau le shell pour détecter le mot "speed=" puis on récupère la valeur qui suit. On limite les valeurs possibles entre 0 et 100 pour éviter certains problèmes puis on effectue le calcul permettant de fournir la valeur correspondantes au PWM. On affiche ensuite la valeur à l'utilisateur. 
