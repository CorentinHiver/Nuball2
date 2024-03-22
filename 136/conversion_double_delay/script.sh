#!/bin/bash

# Définition de la commande à exécuter
multithreaded_command="./main -U -m 10 -p 2 -d"
monothreaded_command="./main -U -m 1 -p 2 -d"
nombre_de_tentatives=0

# Boucle jusqu'à ce que le programme s'exécute avec succès
while true; do
  ((nombre_de_tentatives++)) #Nombre de tentatives
  echo "Launching forced conversion for the &nombre_de_tentatives time..."
  $multithreaded_command # Exécuter la commande

  # Vérifier le code de retour de la commande
  if [ $? -eq 0 ]; then
      echo "conversion finished with success !!"
      break # Sortir de la boucle si la commande s'est terminée avec succès
  else
      echo "conversion failed... Back to monothread for 5 minutes"
      $monothreaded_command &
      background_pid=$!
      sleep 300
      if ps -p $secondary_pid > /dev/null; then
        echo "Sending SIGINT to secondary command..."
        kill -2 $secondary_pid
      fi
  fi
done

echo "Conversion should be finished by now ! only &nombre_de_tentatives tentatives"