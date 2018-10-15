#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <map>
#include <vector>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>

#include "../pugixml/pugixml.hpp"
#include "RuleParser.h"
#include "RuleExecution.h"
#include "TranElemLiterals.h"
#include "CLExec.h"

#include <omp.h>

using namespace std;
using namespace pugi;
using namespace elem;

int
main (void)
{

  string inFilePath, preProcessFilePath = "preProcess.txt", sentenceFilePath =
      "sentences.txt", biltransFilePath = "biltrans.txt", lextorFilePath = "lextor.txt",
      outputFilePath = "default.out", interInFilePath = "interchunkIn.txt",
      interOutFilePath = "interchunkOut.txt", postOutFilePath = "postchunkOut.txt",
      transferOutFilePath = "transferOut.txt", weightOutFilePath = "weightOut.txt",
      beamFilePath = "beamResults.txt";

  cout << "Please enter the corpus file path : " << endl;
  cin >> inFilePath;
  cout << "Please enter the output file path : " << endl;
  cin >> outputFilePath;

  // start timer
  struct timeval stop, start;
  gettimeofday (&start, NULL);

  cout << "Pre-process corpus started" << endl;

  ifstream inFile (inFilePath.c_str ());
  ofstream preProcessFile (preProcessFilePath.c_str ());

  // preprossing
  if (inFile.is_open () && preProcessFile.is_open ())
    {
      string line;
      while (getline (inFile, line))
	{
	  // remove double quotes
	  for (unsigned i = 0; i < line.size (); i++)
	    {
	      if (line[i] == '"')
		{
		  line.erase (i, 1);
		  i--;
		}
	    }
	  preProcessFile << line << endl;
	}

      inFile.close ();
      preProcessFile.close ();
    }
  else
    {
      cout << "No file found!" << endl;
    }
  cout << "Pre-process corpus started" << endl;

  cout << "Breaking corpus into sentences started" << endl;
// then segment the line by sentences
  CLExec::segmenter (preProcessFilePath, sentenceFilePath);
  cout << "Breaking corpus into sentences finished" << endl;

// do the biltrans for all sentences
  cout << "biltrans of all sentences started" << endl;
  CLExec::biltrans (sentenceFilePath, biltransFilePath);
  cout << "biltrans of all sentences finished" << endl;

// do the lextor for all sentences
  cout << "Lextor of all sentences started" << endl;
  CLExec::lextor (biltransFilePath, lextorFilePath);
  cout << "Lextor of all sentences finished" << endl;

  ifstream lextorFile (lextorFilePath.c_str ());
  ifstream inSentenceFile (sentenceFilePath.c_str ());
  ofstream outputFile (outputFilePath.c_str ());
//  ofstream beamFile (beamFilePath.c_str ());

  if (lextorFile.is_open () && outputFile.is_open () && inSentenceFile.is_open ())
//      && beamFile.is_open ())
    {

      // load transfer file in an xml document object
      xml_document transferDoc;
      xml_parse_result result = transferDoc.load_file ("transferFile.t1x");

      if (string (result.description ()) != "No error")
	{
	  cout << "ERROR : " << result.description () << endl;
	  return -1;
	}

      // xml node of the parent node (transfer) in the transfer file
      xml_node transfer = transferDoc.child ("transfer");

      // load yasmet models data
//      map<string, map<string, vector<float> > > classesWeights =
//	  CLExec::loadYasmetModels ();

      vector<string> sourceSentences, tokenizedSentences;

//      long sentId = 1;
      string tokenizedSentence;
      while (getline (lextorFile, tokenizedSentence))
	{
	  string sourceSentence;
	  if (!getline (inSentenceFile, sourceSentence))
	    sourceSentence = "No more sentences";

//	  cout << sentId++ << endl;
//	  cout << "source : " << sourceSentence << endl;
//	  cout << "lextor : " << tokenizedSentence << endl;

	  sourceSentences.push_back (sourceSentence);
	  tokenizedSentences.push_back (tokenizedSentence);
	}

      lextorFile.close ();
      inSentenceFile.close ();

      vector<string> vslTokens[sourceSentences.size ()];
      vector<string> vouts[sourceSentences.size ()];
      vector<vector<unsigned> > vrulesIds[sourceSentences.size ()];
      vector<vector<vector<xml_node> > > voutsRules[sourceSentences.size ()];
      vector<pair<pair<unsigned, unsigned>, pair<unsigned, vector<vector<xml_node> > > > > vambigInfo[sourceSentences.size ()];
      vector<vector<unsigned> > vweigInds[sourceSentences.size ()];

//#pragma omp parallel for
      for (unsigned i = 0; i < sourceSentences.size (); i++)
	{
	  string sourceSentence, tokenizedSentence;
	  sourceSentence = sourceSentences[i];
	  tokenizedSentence = tokenizedSentences[i];

	  // spaces after each token
	  vector<string> spaces;

	  // tokens in the sentence order
	  vector<string> slTokens, tlTokens;

	  // tags of tokens in order
	  vector<vector<string> > slTags, tlTags;

	  RuleParser::sentenceTokenizer (&slTokens, &tlTokens, &slTags, &tlTags, &spaces,
					 tokenizedSentence);

	  // map of tokens ids and their matched categories
	  map<int, vector<string> > catsApplied;

	  RuleParser::matchCats (&catsApplied, slTokens, slTags, transfer);

	  // map of matched rules and tokens id
	  map<xml_node, vector<vector<int> > > rulesApplied;

	  RuleParser::matchRules (&rulesApplied, slTokens, catsApplied, transfer);

	  map<string, vector<vector<string> > > attrs = RuleParser::getAttrs (transfer);

	  // rule and (target) token map to specific output
	  // if rule has many patterns we will choose the first token only
	  map<xml_node, map<int, vector<string> > > ruleOutputs;

	  // map (target) token to all matched rules and the number of pattern items of each rule
	  map<int, vector<pair<xml_node, unsigned> > > tokenRules;

	  RuleExecution::ruleOuts (&ruleOutputs, &tokenRules, tlTokens, tlTags,
				   rulesApplied, attrs);

	  // final outs and their applied rules
	  vector<string> outs;
	  vector<vector<unsigned> > rulesIds;
	  vector<vector<vector<xml_node> > > outsRules;

	  // ambiguity info contains the id of the first token and
	  // the number of the token as a pair and then another pair
	  // contains position of ambiguous rules among other rules and
	  // vector of the rules applied to that tokens
	  vector<
	      pair<pair<unsigned, unsigned>, pair<unsigned, vector<vector<xml_node> > > > > ambigInfo;

	  RuleExecution::outputs (&outs, &rulesIds, &outsRules, &ambigInfo, tlTokens,
				  tlTags, ruleOutputs, tokenRules, spaces);

	  // indexes of accumulated weights
	  vector<vector<unsigned> > weigInds;

	  RuleExecution::weightIndices (&weigInds, ambigInfo, outsRules);

	  vslTokens[i] = slTokens;
	  vouts[i] = outs;
	  vrulesIds[i] = rulesIds;
	  voutsRules[i] = outsRules;
	  vambigInfo[i] = ambigInfo;
	  vweigInds[i] = weigInds;
	}

      ofstream interInFile (interInFilePath.c_str ());
      if (interInFile.is_open ())
	for (unsigned i = 0; i < sourceSentences.size (); i++)
	  {
	    vector<string> outs = vouts[i];
	    for (unsigned j = 0; j < outs.size (); j++)
	      {
		interInFile << outs[j] << endl;
	      }
	  }
      else
	cout << "error in opening interInFile" << endl;
      interInFile.close ();

      CLExec::interchunk (interInFilePath, interOutFilePath);

      CLExec::postchunk (interOutFilePath, postOutFilePath);

      CLExec::transfer (postOutFilePath, transferOutFilePath);

      CLExec::assignWeights (transferOutFilePath, weightOutFilePath);

      vector<string> vtransfers[sourceSentences.size ()];

      ifstream transferOutFile (transferOutFilePath.c_str ());
      if (transferOutFile.is_open ())
	for (unsigned i = 0; i < sourceSentences.size (); i++)
	  {
	    string line;
	    vector<string> outs = vouts[i];
	    vector<string> transfers;
	    for (unsigned j = 0; j < outs.size (); j++)
	      {
		getline (transferOutFile, line);
		transfers.push_back (line);
	      }

	    vtransfers[i] = transfers;
	  }
      else
	cout << "error in opening weightOutFile" << endl;
      transferOutFile.close ();

      vector<float> vweights[sourceSentences.size ()];

      ifstream weightOutFile (weightOutFilePath.c_str ());
      if (weightOutFile.is_open ())
	for (unsigned i = 0; i < sourceSentences.size (); i++)
	  {
	    string line;
	    vector<string> outs = vouts[i];
	    vector<float> weights;
	    float sum = 0;
	    for (unsigned j = 0; j < outs.size (); j++)
	      {
		getline (weightOutFile, line);
		float weight = strtof (line.c_str (), NULL);
		weights.push_back (weight);
		sum += weight;
	      }
	    for (unsigned j = 0; j < outs.size (); j++)
		weights[j] /= sum;

	    vweights[i] = weights;
	  }
      else
	cout << "error in opening weightOutFile" << endl;
      weightOutFile.close ();

      // prepare yasmet datasets
      for (unsigned g = 0; g < sourceSentences.size (); g++)
	{
	  vector<
	      pair<pair<unsigned, unsigned>, pair<unsigned, vector<vector<xml_node> > > > > ambigInfo =
	      vambigInfo[g];

	  vector<vector<unsigned> > weigInds = vweigInds[g];

	  vector<float> weights = vweights[g];

	  vector<string> slTokens = vslTokens[g];

	  vector<vector<unsigned> >::iterator weigIndIt = weigInds.begin ();
	  for (unsigned i = 0; i < ambigInfo.size (); i++)
	    {

	      pair<pair<unsigned, unsigned>, pair<unsigned, vector<vector<xml_node> > > > p1 =
		  ambigInfo[i];
	      pair<unsigned, unsigned> p2 = p1.first;
	      vector<vector<xml_node> > ambigRules = p1.second.second;

	      // name of the file is the concatenation of patterns
	      string rulesNums;
	      for (unsigned x = 0; x < ambigRules.size (); x++)
		{
		  for (unsigned y = 0; y < ambigRules[x].size (); y++)
		    {
		      xml_node rule = ambigRules[x][y];
		      string ruleCmnt = rule.first_attribute ().value ();

		      rulesNums += ruleCmnt.substr (ruleCmnt.find_last_of ("-") + 1);
		      rulesNums += "_";

		    }
		  rulesNums += "+";
		}

	      // if it's the first time to open , put the number of classes
	      bool firstTime = true;
	      if (FILE *file = fopen ((DATASETS + string ("/") + rulesNums).c_str (),
				      "r"))
		{
		  firstTime = false;
		  fclose (file);
		}

	      ofstream dataset ((DATASETS + string ("/") + rulesNums).c_str (),
				ofstream::out | ofstream::app);

	      if (firstTime)
		dataset << ambigRules.size () << endl;

	      for (unsigned x = 0; x < ambigRules.size (); x++)
		{

		  dataset << x << " $ ";

		  vector<unsigned> weigInd = *weigIndIt++;
		  float count = 0;

		  for (unsigned z = 0; z < weigInd.size (); z++)
		    count += weights[weigInd[z]];

		  dataset << count << " #";

		  string features;
		  for (unsigned v = 0; v < ambigRules.size (); v++)
		    {
		      char label[21];
		      sprintf (label, "%d", v);

		      for (unsigned z = p2.first; z < p2.first + p2.second; z++)
			{
			  char num[21];
			  sprintf (num, "%d", z - p2.first);
			  string word = CLExec::toLowerCase (slTokens[z]);
			  for (unsigned c = 0; c < word.length (); c++)
			    if (word[c] == ' ')
			      word.replace (c, 1, "_");

			  features += " " + word + "_" + num + ":" + label;
			}
		      features += " #";
		    }
		  dataset << features << endl;
		}
	      dataset.close ();
	    }
	}

      for (unsigned i = 0; i < sourceSentences.size (); i++)
	{
	  // Write sentence analysis
	  outputFile
	      << "---------------------------------------------------------------------------------------------------------"
	      << endl << endl;
	  outputFile << "Analysis of sentence : " << endl;
	  outputFile << sourceSentences[i] << endl << endl << endl;

	  outputFile << endl;
	  outputFile << "sentence id ||| coverage id ||| original sentence |||"
	      << " lextor ||| rules ||| chunker ||| final sentence ||| score" << endl
	      << endl;

	  for (unsigned j = 0; j < vweights[i].size (); j++)
	    {
	      // sentence id
	      outputFile << (i + 1) << " ||| ";
	      // coverage id
	      outputFile << (j + 1) << " ||| ";
	      // original sentence
	      outputFile << sourceSentences[i] << " ||| ";
	      // lextor
	      outputFile << tokenizedSentences[i] << " ||| ";
	      // rules
	      for (unsigned k = 0; k < vrulesIds[i][j].size (); k++)
		outputFile << vrulesIds[i][j][k] << " ";
	      outputFile << "||| ";
	      // chuncker
	      outputFile << vouts[i][j] << " ||| ";
	      // final sentence
	      outputFile << vtransfers[i][j] << " ||| ";
	      // score
	      outputFile << vweights[i][j] << endl << endl;
	    }
	}
      outputFile.close ();
    }
  else
    {
      cout << "ERROR in opening files!" << endl;
    }

  cout << endl << endl << "-----------------------------------------------------" << endl
      << endl;
  cout << "Finished" << endl;

  gettimeofday (&stop, NULL);
  cout << "\nTime taken :  "

  << (stop.tv_sec - start.tv_sec + (stop.tv_usec - start.tv_usec) / 1000000.0)
      << "  seconds" << endl;

}
