#!/bin/bash

servers=("mteverest1" "mteverest2" )

username="abhattar"


bash master_start_other_machines.sh "${servers[@]}" "${username}"