#!/bin/bash

# Définition de la commande à exécuter
executable=./main
multithreaded_command="$executable -U -m 10 -p 2 -d"
monothreaded_command="$executable -U -m 1 -p 2 -d"
nombre_de_tentatives=0

# Boucle jusqu'à ce que le programme s'exécute avec succès
while true; do
  nombre_de_tentatives = $((nombre_de_tentatives + 1)) #Nombre de tentatives
  echo "Launching forced conversion for the &nombre_de_tentatives time..."

  #If no access to the executable, make it again :
  if ! [ -x "$executable" ]; then
    make opt
    #if the make failed, exit :
    if [ $? -eq 0 ]; then
      echo "Make completed successfully."
    else
      echo "Make encountered an error."
    fi
  fi

  #launch the command :
  $multithreaded_command 

  # Vérifier le code de retour de la commande
  if [ $? -eq 0 ]; then
      echo "conversion finished with success !!"
      break # Sortir de la boucle si la commande s'est terminée avec succès
  else
    echo "conversion failed... Back to monothread for 5 minutes"
    #If no access to the executable, make it again :
    if ! [ -x "$executable" ]; then
      make opt
      #if the make failed, exit :
      if [ $? -eq 0 ]; then
        echo "Make completed successfully."
      else
        echo "Make encountered an error."
      fi
    fi

    # launch the executable with multithread deactivated
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