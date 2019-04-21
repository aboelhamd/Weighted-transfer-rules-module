#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include <sstream>

#include "../pugixml/pugixml.hpp"
#include "RuleParser.h"
#include "RuleExecution.h"
#include "TranElemLiterals.h"
#include "CLExec.h"

using namespace std;
using namespace pugi;
using namespace elem;

int
main (int argc, char **argv)
{
  string localeId, transferFilePath, lextorFilePath, interInFilePath;

  if (argc == 5)
    {
      localeId = argv[1];
      transferFilePath = argv[2];
      lextorFilePath = argv[3];
      interInFilePath = argv[4];
    }
  else
    {
//      localeId = "es_ES";
//      transferFilePath = "transferFile.t1x";
//      sentenceFilePath = "spa-test.txt";
//      lextorFilePath = "spa-test.lextor";
//      interInFilePath = "inter2.txt";

//      localeId = "kk_KZ";
//      transferFilePath = "apertium-kaz-tur.kaz-tur.t1x";
//      sentenceFilePath = "sample-sentences.txt";
//      lextorFilePath = "sample-lextor.txt";
//      interInFilePath = "sample-inter.txt";

      localeId = "es_ES";
      transferFilePath =
	  "/home/aboelhamd/apertium-eng-spa-ambiguous-rules/apertium-eng-spa.spa-eng.t1x";
      lextorFilePath = "/home/aboelhamd/Downloads/es-en/splits/xaa-lextor.txt";
      interInFilePath = "/home/aboelhamd/Downloads/es-en/splits/xaa-chunker.txt";

      cout << "Error in parameters !" << endl;
      cout
	  << "Parameters are : localeId transferFilePath sentenceFilePath lextorFilePath interInFilePath"
	  << endl;
      cout << "localeId : ICU locale ID for the source language. For Kazakh => kk-KZ"
	  << endl;
      cout << "transferFilePath : Apertium transfer file of the language pair used."
	  << endl;
      cout << "lextorFilePath : Apertium lextor file for the source language sentences."
	  << endl;
      cout
	  << "interInFilePath : Output file name of this program which is the input for apertium interchunk."
	  << endl;
      return -1;
    }

  ifstream lextorFile (lextorFilePath.c_str ());
  ofstream interInFile (interInFilePath.c_str ());
  if (lextorFile.is_open () && interInFile.is_open ())
    {
      // load transfer file in an xml document object
      xml_document transferDoc;
      xml_parse_result result = transferDoc.load_file (transferFilePath.c_str ());

      if (string (result.description ()) != "No error")
	{
	  cout << "ERROR : " << result.description () << endl;
	  return -1;
	}

      // xml node of the parent node (transfer) in the transfer file
      xml_node transfer = transferDoc.child ("transfer");

      map<string, vector<vector<string> > > attrs = RuleParser::getAttrs (transfer);
      map<string, string> vars = RuleParser::getVars (transfer);
      map<string, vector<string> > lists = RuleParser::getLists (transfer);

//      unsigned i = 0;
      string tokenizedSentence;
      while (getline (lextorFile, tokenizedSentence))
	{
//	  cout << i++ << endl;

	  // spaces after each token
	  vector<string> spaces;

	  // tokens in the sentence order
	  vector<string> slTokens, tlTokens;

	  // tags of tokens in order
	  vector<vector<string> > slTags, tlTags;

	  RuleParser::sentenceTokenizer (&slTokens, &tlTokens, &slTags, &tlTags, &spaces,
					 tokenizedSentence);

	  // map of tokens ids and their matched categories
	  map<unsigned, vector<string> > catsApplied;

	  RuleParser::matchCats (&catsApplied, slTokens, slTags, transfer);

	  // map of matched rules and a pair of first token id and patterns number
	  map<xml_node, vector<pair<unsigned, unsigned> > > rulesApplied;

	  RuleParser::matchRules (&rulesApplied, slTokens, catsApplied, transfer);

	  // rule and (target) token map to specific output
	  // if rule has many patterns we will choose the first token only
	  map<unsigned, map<unsigned, string> > ruleOutputs;

	  // map (target) token to all matched rules ids and the number of pattern items of each rule
	  map<unsigned, vector<pair<unsigned, unsigned> > > tokenRules;

	  RuleExecution::ruleOuts (&ruleOutputs, &tokenRules, slTokens, slTags, tlTokens,
				   tlTags, rulesApplied, attrs, lists, &vars, spaces,
				   localeId);
	  // final outs
	  vector<string> outs;
	  // number of possible combinations
	  unsigned compNum;
	  // nodes for every token and rule
	  map<unsigned, vector<RuleExecution::Node*> > nodesPool;
	  // ambiguous informations
	  vector<RuleExecution::AmbigInfo*> ambigInfo;

	  // rules combinations
	  vector<vector<RuleExecution::Node*> > combNodes;

	  nodesPool = RuleExecution::getNodesPool (tokenRules);

	  RuleExecution::getAmbigInfo (tokenRules, nodesPool, &ambigInfo, &compNum);
	  RuleExecution::getOuts (&outs, &combNodes, ambigInfo, nodesPool, ruleOutputs,
				  spaces);

//	  for (unsigned j = 0; j < tlTokens.size (); j++)
//	    {
//	      cout << tlTokens[j] << endl;
//	      vector<pair<unsigned, unsigned> > rulees = tokenRules[j];
//	      for (unsigned k = 0; k < rulees.size (); k++)
//		{
//		  cout << rulees[k].first << " , " << rulees[k].second << endl;
//		}
//	      cout << endl;
//	    }
//
//	  for (unsigned j = 0; j < ambigInfo.size (); j++)
//	    {
//	      cout << "firTokId = " << ambigInfo[j]->firTokId << "; maxPat = "
//		  << ambigInfo[j]->maxPat << endl;
//	      vector<vector<RuleExecution::Node*> > combinations =
//		  ambigInfo[j]->combinations;
//	      cout << endl;
//	      for (unsigned k = 0; k < combinations.size (); k++)
//		{
//		  vector<RuleExecution::Node*> nodes = combinations[k];
//		  for (unsigned l = 1; l < nodes.size (); l++)
//		    {
//		      cout << "tok=" << nodes[l]->tokenId << "; rul=" << nodes[l]->ruleId
//			  << "; pat=" << nodes[l]->patNum << " - ";
//		    }
//		  cout << endl;
//		}
//	      cout << endl;
//	    }
//
//	  for (map<unsigned, map<unsigned, string> >::iterator it = ruleOutputs.begin ();
//	      it != ruleOutputs.end (); it++)
//	    {
//	      cout << "ruleId=" << it->first << endl;
//	      map<unsigned, string> outs = it->second;
//
//	      for (map<unsigned, string>::iterator it2 = outs.begin ();
//		  it2 != outs.end (); it2++)
//		{
//		  cout << "tokId=" << it2->first << " , out = " << it2->second << endl;
//		}
//	      cout << endl;
//	    }
//	  cout << endl;
//
//	  for (unsigned j = 0; j < tlTokens.size (); j++)
//	    {
//	      vector<RuleExecution::Node*> nodes = nodesPool[j];
//	      cout << "tokId = " << j << " : " << tlTokens[j] << endl;
//	      for (unsigned k = 0; k < nodes.size (); k++)
//		{
//		  cout << "ruleId = " << nodes[k]->ruleId << "; patNum = "
//		      << nodes[k]->patNum << endl;
//		}
//	      cout << endl;
//	    }
//
//	  for (unsigned j = 0; j < combNodes.size (); j++)
//	    {
//	      vector<RuleExecution::Node*> nodes = combNodes[j];
//	      for (unsigned k = 0; k < nodes.size (); k++)
//		{
//		  cout << "tok=" << nodes[k]->tokenId << "; rul=" << nodes[k]->ruleId
//		      << "; pat=" << nodes[k]->patNum << " - ";
//		}
//	      cout << endl;
//	    }

	  // write the outs
	  for (unsigned j = 0; j < outs.size (); j++)
	    interInFile << outs[j] << endl;

	  // delete AmbigInfo pointers
	  for (unsigned j = 0; j < ambigInfo.size (); j++)
	    {
	      // delete the dummy node pointers
	      set<RuleExecution::Node*> dummies;
	      for (unsigned k = 0; k < ambigInfo[j]->combinations.size (); k++)
		dummies.insert (ambigInfo[j]->combinations[k][0]);
	      for (set<RuleExecution::Node*>::iterator it = dummies.begin ();
		  it != dummies.end (); it++)
		delete (*it);

	      delete ambigInfo[j];
	    }
	  // delete Node pointers
	  for (map<unsigned, vector<RuleExecution::Node*> >::iterator it =
	      nodesPool.begin (); it != nodesPool.end (); it++)
	    {
	      for (unsigned j = 0; j < it->second.size (); j++)
		{
		  delete it->second[j];
		}
	    }
	}

      lextorFile.close ();
      interInFile.close ();
      cout << "RulesApplier finished!";
    }
  else
    {
      cout << "ERROR in opening files!" << endl;
    }

  return 0;
}
