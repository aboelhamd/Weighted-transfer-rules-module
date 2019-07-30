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

if (len(sys.argv) != 3) :
	print('\nUsage: python3 sklearn-train.py datasets_path models_path');
	sys.exit()

dataset_path = sys.argv[1]
models_path = sys.argv[2]

files = {}
# r=root, d=directories, f=files
for r, d, f in os.walk(path):
  for file in f:
    files[file]=os.path.join(r, file)

if not os.path.exists(models_path):
  os.makedirs(models_path)


# These are the classifiers that permit training data with sample weights!
for file in files:

  models_names = ["NaiveBayes", "LinearSVM", "RBFSVM", "DecisionTree",
       "RandomForest", "AdaBoost"]

  classifiers = [
      GaussianNB(),
      SVC(kernel="linear", C=0.025),
      SVC(gamma=2, C=1),
      DecisionTreeClassifier(max_depth=5),
      RandomForestClassifier(max_depth=5, n_estimators=10, max_features=1),
      AdaBoostClassifier()]
  
  print("file name :", file)
  data = pd.read_csv(files[file], delimiter=r"\s+").dropna()
  
#     print (data.shape[0] , data.iloc[:,0].nunique())
  # if records equals to classes number, duplicates the data  
  if data.shape[0] == data.iloc[:,0].nunique():
    data = data.append(data)
#    display(data)
  
#    print(data.iloc[:,2:])

  # words (features) encoding
  from sklearn.preprocessing import OrdinalEncoder
  enc = OrdinalEncoder(dtype=np.int32)
  features = enc.fit_transform(data.iloc[:,2:])
  
  # save the encoder
  enc_name = models_path+'/'+'encoder'+'-'+file[:-4]
  joblib.dump(enc, enc_name)
#     display(enc.categories_)
#     display(data.iloc[:,2:],features)
  # target and weights
  target = data.iloc[:,0]
  weights = data.iloc[:,1].values
  
#     print("file name :", file)
  print("Rules(classes) number :",target.nunique())
  print("Words(features) number :",features.shape[1])
  print("Records number :",features.shape[0], end = '')
  display(data.iloc[:target.nunique(),:])
  
  # split to train and test
  X_train, X_test, y_train, y_test, w_train, w_test = \
      train_test_split(features, target, weights, test_size=.5, random_state=0, stratify=target)
#     display(features, target, weights)
#     display(X_train, X_test, y_train, y_test, w_train, w_test)
  
  # train models and print their scores
  for name, model in zip(models_names, classifiers):
    print("model :", name, ",", end = '')
    model.fit(X=X_train, y=y_train, sample_weight=w_train)
    score = model.score(X=X_test, y=y_test, sample_weight=w_test)
    print(" score =", score)
    
    # save models
    model_name = models_path+'/'+name+'-'+file[:-4]
    joblib.dump(model, model_name)
  print("----------------------------------------------\n")

