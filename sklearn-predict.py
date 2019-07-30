import os
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
from matplotlib.colors import ListedColormap
from sklearn.model_selection import train_test_split
from sklearn.svm import SVC
from sklearn.tree import DecisionTreeClassifier
from sklearn.ensemble import RandomForestClassifier, AdaBoostClassifier
from sklearn.naive_bayes import GaussianNB
from sklearn.externals import joblib
from sklearn.preprocessing import OrdinalEncoder

if (len(sys.argv) != 5) :
	print('\nUsage: python3 sklearn-predict.py data_file models_path model_name output_file');
	sys.exit()

data_path = sys.argv[1]
models_path = sys.argv[2]
model_name = sys.argv[3]
output_path = sys.argv[4]

data = open(data_path, 'r')
output = open(output_path, 'w+')


# These are the classifiers that permit training data with sample weights!
# ["NaiveBayes", "LinearSVM", "RBFSVM", "DecisionTree", "RandomForest", "AdaBoost"]

for line in data :
  file_name = line.split(' ')[0]
  features = line.split(' ')[1:]

  # load the features encoder
  enc = joblib.load(models_path+'/'+'encoder'+'-'+file_name)

  # see if features are seen before or not
  seen = True
  for i in range (features) :
    if features[i] not enc.categories_[i] :
      seen = False
      break

  if seen :
    features = enc.transform(features)
    # load the model
    loaded_model = joblib.load(models_path+'/'+name+'-'+file_name)
    output.write(loaded_model.predict([features])+'\n')

  else :
    output.write('0\n')
