#!/bin/bash

# Définition de la commande à exécuter
commande_a_executer="./main -U -m 10 -p 2 -d"
nombre_de_tentatives=0

# Boucle jusqu'à ce que le programme s'exécute avec succès
while true; do
  nombre_de_tentatives=$((nombre_de_tentatives + 1)) #Nombre de tentatives
  echo "Launching forced conversion..."
  $commande_a_executer # Exécuter la commande

  # Vérifier le code de retour de la commande
  if [ $? -eq 0 ]; then
      echo "conversion finished with success !!"
      break # Sortir de la boucle si la commande s'est terminée avec succès
  else
      echo "conversion failed... Waiting 60 seconds before restart"
      sleep 15 # Attendre 30 secondes avant de relancer la commande
      make opt # build the executable in case the source cod ehas changed
      sleep 15 # Attendre 30 secondes avant de relancer la commande
  fi
done

echo "Conversion should be finished by now ! only &nombre_de_tentatives tentatives"