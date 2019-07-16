#!/bin/bash

# datasets folder path
yasmet=$1
# models folder path
sklearn=$2

mkdir $sklearn

for dataset in $yasmet/*;
do
   python3 yasmet-to-sklearn.py $dataset $sklearn/${dataset:${#yasmet}+1}.csv;
done
