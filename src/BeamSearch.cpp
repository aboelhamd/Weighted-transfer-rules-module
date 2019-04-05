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
#include <sstream>

#include "RuleParser.h"
#include "TranElemLiterals.h"
#include "CLExec.h"
#include "BeamSearch.h"

#include <omp.h>
#include "ambiguous_transfer.h"

using namespace std;
using namespace elem;

void
BeamSearch::transfer (string transferFilePath, string modelsFileDest, string k,
		      FILE* lextorFile, FILE* outFile)
{

  xmlDoc* doc = xmlReadFile (transferFilePath.c_str (), NULL, 0);

  if (doc == NULL)
    {
      cerr << "Error: Could not parse file \'" << transferFilePath << "\'." << endl;
      exit (EXIT_FAILURE);
    }

  xmlNode* transfer = xmlDocGetRootElement (doc);

  map<string, vector<vector<string> > > attrs = RuleParser::getAttrs (transfer);

//  cout << "size = " << attrs.size () << endl;
//  for (map<string, vector<vector<string> > >::iterator it = attrs.begin ();
//      it != attrs.end (); it++)
//    {
//      cout << "attr-item : " << it->first << endl;
//      vector<vector<string> > items = it->second;
//      for (unsigned i = 0; i < items.size (); i++)
//	{
//	  for (unsigned j = 0; j < items[i].size (); j++)
//	    {
//	      cout << items[i][j] << " ";
//	    }
//	  cout << endl;
//	}
//      cout << endl;
//    }

  map<string, string> vars = RuleParser::getVars (transfer);
  map<string, vector<string> > lists = RuleParser::getLists (transfer);

  string localeId;
  map<string, map<string, vector<float> > > classesWeights = CLExec::loadYasmetModels (
      modelsFileDest, &localeId);

  int beam;
  stringstream buffer (k);
  buffer >> beam;

  char buff[10240];
  string tokenizedSentence;
  while (fgets (buff, 10240, lextorFile))
    {
      tokenizedSentence = buff;

      // spaces after each token
      vector<string> spaces;

      // tokens in the sentence order
      vector<string> slTokens, tlTokens;

      // tags of tokens in order
      vector<vector<string> > slTags, tlTags;

//      cout << "here1" << endl;
      RuleParser::sentenceTokenizer (&slTokens, &tlTokens, &slTags, &tlTags, &spaces,
				     tokenizedSentence);
//      cout << "here2" << endl;
      // map of tokens ids and their matched categories
      map<unsigned, vector<string> > catsApplied;

      RuleParser::matchCats (&catsApplied, slTokens, slTags, transfer);
//      cout << "size = " << catsApplied.size () << endl;
//      for (map<unsigned, vector<string> >::iterator it = catsApplied.begin ();
//	  it != catsApplied.end (); it++)
//	{
//	  cout << "tokId : " << it->first << endl;
//	  vector<string> cats = it->second;
//	  for (unsigned i = 0; i < cats.size (); i++)
//	    {
//	      cout << cats[i] << " ";
//	    }
//	  cout << endl;
//	}

      //      cout << "here3" << endl;
      // map of matched rules and a pair of first token id and patterns number
      map<xmlNode*, vector<pair<unsigned, unsigned> > > rulesApplied;

      RuleParser::matchRules (&rulesApplied, slTokens, catsApplied, transfer);

//      cout << "size = " << rulesApplied.size () << endl;
//      for (map<xmlNode*, vector<pair<unsigned, unsigned> > >::iterator it =
//	  rulesApplied.begin (); it != rulesApplied.end (); it++)
//	{
//	  cout << "ruleId : " << xml_operations::getAttValUnsg (it->first, string ("id"))
//	      << endl;
////	  vector<string> cats = it->second;
////	  for (unsigned i = 0; i < cats.size (); i++)
////	    {
////	      cout << cats[i] << " ";
////	    }
////	  cout << endl;
//	}

//      cout << "here4" << endl;
      // rule and (target) token map to specific output
      // if rule has many patterns we will choose the first token only
      map<unsigned, map<unsigned, string> > ruleOutputs;

      // map (target) token to all matched rules ids and the number of pattern items of each rule
      map<unsigned, vector<pair<unsigned, unsigned> > > tokenRules;

      ambiguous_transfer::ruleOuts (&ruleOutputs, &tokenRules, slTokens, slTags, tlTokens,
				    tlTags, rulesApplied, attrs, lists, &vars, spaces,
				    localeId);
//      cout << "here5" << endl;
      // final outputs
      vector<string> outs;
      // number of generated combinations
      unsigned compNum;
      // nodes for every token and rule
      map<unsigned, vector<ambiguous_transfer::Node> > nodesPool;
      // ambiguous informations
      vector<ambiguous_transfer::AmbigInfo> ambigInfo;
      // beam tree
      vector<pair<vector<ambiguous_transfer::Node>, float> > beamTree;
      // rules combinations
      vector<vector<ambiguous_transfer::Node> > combNodes;

      nodesPool = ambiguous_transfer::getNodesPool (tokenRules);
//      cout << "here6" << endl;
      ambiguous_transfer::getAmbigInfo (tokenRules, nodesPool, &ambigInfo, &compNum);
//      cout << "ambigsize = " << ambigInfo.size () << endl;
      vector<ambiguous_transfer::AmbigInfo> newAmbigInfo;
      for (unsigned j = 0; j < ambigInfo.size (); j++)
	if (ambigInfo[j].combinations.size () > 1)
	  newAmbigInfo.push_back (ambigInfo[j]);

//      cout << "here8" << endl;
      CLExec::beamSearch (&beamTree, beam, slTokens, newAmbigInfo, classesWeights,
			  localeId);
//      cout << "here9" << endl;
      ambiguous_transfer::getOuts (&outs, &combNodes, beamTree, nodesPool, ruleOutputs,
				   spaces);
//      cout << "here8" << endl;

      for (map<unsigned, map<unsigned, string> >::iterator it = ruleOutputs.begin ();
	  it != ruleOutputs.end (); it++)
	{
	  cout << "ruleId=" << it->first << endl;
	  map<unsigned, string> outs = it->second;

	  for (map<unsigned, string>::iterator it2 = outs.begin (); it2 != outs.end ();
	      it2++)
	    {
	      cout << "tokId=" << it2->first << " , out = " << it2->second << endl;
	    }
	  cout << endl;
	}
      cout << endl;

      for (unsigned j = 0; j < tlTokens.size (); j++)
	{
	  vector<ambiguous_transfer::Node> nodes = nodesPool[j];
	  cout << "tokId = " << j << " : " << tlTokens[j] << endl;
	  for (unsigned k = 0; k < nodes.size (); k++)
	    {
	      cout << "ruleId = " << nodes[k].ruleId << "; patNum = " << nodes[k].patNum
		  << endl;
	    }
	  cout << endl;
	}

      for (unsigned j = 0; j < combNodes.size (); j++)
	{
	  vector<ambiguous_transfer::Node> nodes = combNodes[j];
	  for (unsigned k = 0; k < nodes.size (); k++)
	    {
	      cout << "tok=" << nodes[k].tokenId << "; rul=" << nodes[k].ruleId
		  << "; pat=" << nodes[k].patNum << " - ";
	    }
	  cout << endl;
	}

      for (unsigned j = 0; j < outs.size (); j++)
	cout << outs[j] << endl;

      // write the outs
      for (unsigned j = 0; j < outs.size (); j++)
	{
	  fputs (outs[j].c_str (), outFile);
	}
    }

}

FILE *
open_input (string const &filename)
{
  FILE *input = fopen (filename.c_str (), "r");
  if (!input)
    {
      wcerr << "Error: can't open input file '";
      wcerr << filename.c_str () << "'." << endl;
      exit (EXIT_FAILURE);
    }

  return input;
}

FILE *
open_output (string const &filename)
{
  FILE *output = fopen (filename.c_str (), "w");
  if (!output)
    {
      wcerr << "Error: can't open output file '";
      wcerr << filename.c_str () << "'." << endl;
      exit (EXIT_FAILURE);
    }
  return output;
}

int
main (int argc, char **argv)
{
  string transferFilePath =
      "/home/aboelhamd/eclipse-workspace/machinetranslation/apertium-eng-spa.spa-eng.t1x";
  string modelsFileDest = "nomodel";
  string k = "8";
  string lextor = "lex1.txt";
  string out = "here.txt";

  FILE *input = stdin, *output = stdout;
  input = open_input (lextor);
  output = open_output (out);
  BeamSearch::transfer (transferFilePath, modelsFileDest, k, input, output);
}
