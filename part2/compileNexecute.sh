#!/bin/bash
./compile.sh

if [ $# -eq 0 ]
then
  ./detectionCercle
elif [ $# -eq 1 ]
then
  ./detectionCercle $1
elif [ $# -eq 2 ]
then
  ./detectionCercle $1 $2
elif [ $# -eq 3 ]
then
  ./detectionCercle $1 $2 $3
elif [ $# -eq 4 ]
then
  ./detectionCercle $1 $2 $3 $4
elif [ $# -eq 5 ]
then
  ./detectionCercle $1 $2 $3 $4 $5 
elif [ $# -eq 6 ]
then
  ./detectionCercle $1 $2 $3 $4 $5 $6
elif [ $# -eq 7 ]
then
  ./detectionCercle $1 $2 $3 $4 $5 $6 $7
elif [ $# -eq 8 ]
then
  ./detectionCercle $1 $2 $3 $4 $5 $6 $7 $8
elif [ $# -eq 9 ]
then
  ./detectionCercle $1 $2 $3 $4 $5 $6 $7 $8 $9
fi
