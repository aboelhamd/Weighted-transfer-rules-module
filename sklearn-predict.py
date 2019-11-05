import os
import sys
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
from sklearn.model_selection import train_test_split
from sklearn.svm import SVC, LinearSVC
from sklearn.tree import DecisionTreeClassifier
from sklearn.ensemble import RandomForestClassifier, AdaBoostClassifier
from sklearn.naive_bayes import GaussianNB
from sklearn.externals import joblib
from sklearn.preprocessing import OrdinalEncoder

if (len(sys.argv) != 5) :
	print('\nUsage: python3 sklearn-predict.py data_file models_path model_name output_file\n\model_name: one of linear, poly, rbf, sigmoid, or linearsvc "which trains an OVA linear model".');
	sys.exit()

data_path = sys.argv[1]
models_path = sys.argv[2]
model_name = sys.argv[3].casefold()
output_path = sys.argv[4]

data = open(data_path, 'r')
output = open(output_path, 'w+')


# These are the classifiers that permit training data with sample weights!
# ["NaiveBayes", "LinearSVM", "RBFSVM", "DecisionTree", "RandomForest", "AdaBoost"]

for line in data :
  split = line.split(' ')
  file_name = split[0]
  features = split[1:len(split)-1]
  
  file_no_ext = file_name
  if (file_no_ext.find('.') != -1) :
    file_no_ext = file_no_ext[:file_no_ext.find('.')]

  enc_name = os.path.join(models_path, 'encoder'+'-'+file_no_ext)[:256]
  if os.path.exists(enc_name) :
    # load the features encoder
    enc = joblib.load(enc_name)

    # see if features are seen before or not
    seen = True
    for i in range (len(features)) :
      #print(i, features, enc.categories_)
      if features[i] not in enc.categories_[i] :
        seen = False
        break

    if seen :
      # encode words
      features = enc.transform([features])
      
      # load the model
      name = os.path.join(models_path, model_name+'-'+file_no_ext)[:256]
      loaded_model = joblib.load(name)

      # predict and write in file
      #print('prediction = ', loaded_model.predict(features)[0])
      output.write(str(loaded_model.predict(features)[0]))
      output.write('\n')

    else :
      print("Words : "+str(features)+", are not found in "+file_name)
      output.write('0\n')

  else :
    print("Model "+file_name+" is not trained yet.")
    output.write('0\n')
