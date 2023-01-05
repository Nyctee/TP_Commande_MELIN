# TP systèmes d'acquisition et de commande
TP MSC 2022 MÉLIN Julien 

# Commande basique de la MCC
## Génération de 4 PWM
On souhaite générer à l'aide du Timer1 4 PMW en complémentaires décalée pour contrôler en boucle ouverte le moteur.
Pour se faire elles devront respecter les caractéristiques suivantes:

-Une fréquence de 16khz

-Un temps mort d'au moins 2us

-Résolution minimum de 10bits

On utilise alors la configuration suivante pour le Timer1
![Confiig_tim1](https://user-images.githubusercontent.com/46086052/210862710-4ef70d75-28a2-4dd0-aa01-8efedabad756.jpg)

On retrouve bien les 4PMW en sortie (CH1/CH1N et CH2/CH2N) et avec les valeurs choisies du PSC et de l'ARR 
on obtient bien les 16kHz à partir de l'horloge à 170Mhz. Pour la valeur du DeadTime il faut remplir le registre correspondant.
Avec une valeur de 210 on à bien un DeadTime supérieur à 2us.

#Prise en main du hacheur 

Pour piloter le moteur il nous faudrat dans un premier temps cabler les PWM comme suit:

-Commande YTop(CH1) sur le pin 12 et Ybot(CHI1N) sur le pin 30 du hacheur 

-Commande RTop(CH2) sur le pin 13 et RBot(CH2N) sur le pin 31.

Il faudra aussi fournir le signal ISO_RESET sur le pin 33 du hacheur pour autoriser le démarrage du moteur. 
