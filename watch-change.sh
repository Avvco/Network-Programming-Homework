#!/bin/bash
 
if [ -z "$(which inotifywait)" ]; then
    echo "inotifywait not installed."
    echo "In most distros, it is available in the inotify-tools package."
    exit 1
fi
 
counter=0;
 
function execute() {
    #counter=$((counter+1))
    if [ $counter != 0 ]; then
      echo "Detected change..."
      eval "./reloadDocker.sh"
      counter=0
    else
      counter=1
    fi
}
 
inotifywait --recursive --monitor --format "%e %w%f" \
--event modify,move,create,delete ./ \
| while read changed; do
  if [[ $changed = *./src* || $changed = *./include* ]]
  then
    execute "$@"
  fi
done 

# ./watch-change.sh echo "Running our command..."