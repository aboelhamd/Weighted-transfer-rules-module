/*
 * ambiguous_transfer.cpp
 *
 *  Created on: May 5, 2018
 *      Author: aboelhamd
 */

#include "ambiguous_transfer.h"
#include "CLExec.h"

using namespace std;
using namespace elem;

ambiguous_transfer::Node
ambiguousGraph (map<unsigned, vector<pair<unsigned, unsigned> > > tokenRules,
		map<unsigned, vector<ambiguous_transfer::Node> > nodesPool,
		unsigned firTok, unsigned maxPat)
{
  for (unsigned i = firTok; i < firTok + maxPat; i++)
    {
      vector<ambiguous_transfer::Node> nodes = nodesPool[i];
      for (unsigned j = 0; j < nodes.size (); j++)
	{
	  ambiguous_transfer::Node node = nodes[j];
	  // last nodes will point to nothing
	  if (node.tokenId + node.patNum < firTok + maxPat)
	    node.neighbors = nodesPool[node.tokenId + node.patNum];

	  nodes[j] = node;
	}
      nodesPool[i] = nodes;
    }

  // root(dummy) node points to the first token node/s
  ambiguous_transfer::Node root = ambiguous_transfer::Node (-1, -1, -1);
  root.neighbors = nodesPool[firTok];
  return root;
}

ambiguous_transfer::Node
ambiguousGraph (map<unsigned, vector<pair<unsigned, unsigned> > > tokenRules,
		map<unsigned, vector<ambiguous_transfer::Node> > nodesPool)
{
  for (unsigned i = 0; i < nodesPool.size (); i++)
    {
      vector<ambiguous_transfer::Node> nodes = nodesPool[i];
      for (unsigned j = 0; j < nodes.size (); j++)
	{
	  ambiguous_transfer::Node node = nodes[j];
	  // last nodes will point to not existent nodes
	  if (nodesPool.count (node.tokenId + node.patNum))
	    node.neighbors = nodesPool[node.tokenId + node.patNum];

	  nodes[j] = node;
	}
      nodesPool[i] = nodes;
    }

  // root(dummy) node points to the first token node/s
  ambiguous_transfer::Node root = ambiguous_transfer::Node (-1, -1, -1);
  root.neighbors = nodesPool[0];
  return root;
}

void
ambiguous_transfer::getOuts (
    vector<string>* finalOuts, vector<vector<ambiguous_transfer::Node> >* finalCombNodes,
    vector<pair<vector<ambiguous_transfer::Node>, float> > beamTree,
    map<unsigned, vector<ambiguous_transfer::Node> > nodesPool,
    map<unsigned, map<unsigned, string> > ruleOutputs, vector<string> spaces)
{
  for (unsigned i = 0; i < beamTree.size (); i++)
    {
      map<unsigned, ambiguous_transfer::Node> bestNodes;
      for (unsigned j = 0; j < beamTree[i].first.size (); j++)
	{
	  bestNodes[beamTree[i].first[j].tokenId] = beamTree[i].first[j];
	}

      vector<ambiguous_transfer::Node> nodes;
      string out;
      for (unsigned j = 0; j < nodesPool.size ();)
	{
	  ambiguous_transfer::Node node;
	  if (bestNodes.count (j))
	    node = bestNodes[j];
	  else
	    node = nodesPool[j][0];

	  out += ruleOutputs[node.ruleId][node.tokenId]
	      + spaces[node.tokenId + node.patNum - 1];

	  nodes.push_back (node);

	  j += node.patNum;
	}

      finalCombNodes->push_back (nodes);
      finalOuts->push_back (out);
    }
}

void
ambiguous_transfer::getOuts (vector<string>* finalOuts,
			     vector<vector<ambiguous_transfer::Node> >* finalCombNodes,
			     vector<ambiguous_transfer::AmbigInfo> ambigInfo,
			     map<unsigned, vector<ambiguous_transfer::Node> > nodesPool,
			     map<unsigned, map<unsigned, string> > ruleOutputs,
			     vector<string> spaces)
{
  map<unsigned, ambiguous_transfer::AmbigInfo> ambigMap;
  for (unsigned i = 0; i < ambigInfo.size (); i++)
    {
      ambigMap.insert (
	  pair<unsigned, ambiguous_transfer::AmbigInfo> (ambigInfo[i].firTokId,
							 ambigInfo[i]));
    }

  for (unsigned i = 0; (i < ambigInfo.size ()) || (i < 1); i++)
    {
      vector<vector<ambiguous_transfer::Node> > combNodes;
      combNodes.push_back (vector<ambiguous_transfer::Node> ());
      vector<string> outs;
      outs.push_back ("");

      for (unsigned j = 0; j < nodesPool.size ();)
	{
	  vector<ambiguous_transfer::Node> nodes = nodesPool[j];

	  if (nodes.size () > 1 && ambigMap.count (j))
	    {
	      vector<vector<ambiguous_transfer::Node> > combinations =
		  ambigMap[j].combinations;

	      if (ambigInfo[i].firTokId == j)
		{
		  combNodes = putCombinations (combNodes, combinations);

		  vector<string> ambigOuts;

		  for (unsigned k = 0; k < combinations.size (); k++)
		    {
		      string ambigOut;
		      // skip the dummy node
		      for (unsigned l = 1; l < combinations[k].size (); l++)
			{
			  ambigOut +=
			      ruleOutputs[combinations[k][l].ruleId][combinations[k][l].tokenId]
				  + spaces[combinations[k][l].tokenId
				      + combinations[k][l].patNum - 1];
			}
		      ambigOuts.push_back (ambigOut);
		    }

		  outs = putOuts (outs, ambigOuts);
		}
	      else
		{
		  putCombination (
		      &combNodes,
		      vector<ambiguous_transfer::Node> (combinations[0].begin () + 1,
							combinations[0].end ()));
		  // take the first combination only , while solving the last space issue
		  string ambigOut;
		  // skip the dummy node
		  unsigned l = 1;
		  for (; l < combinations[0].size () - 1; l++)
		    {
		      ambigOut +=
			  ruleOutputs[combinations[0][l].ruleId][combinations[0][l].tokenId]
			      + spaces[combinations[0][l].tokenId
				  + combinations[0][l].patNum - 1];
		    }
		  ambigOut +=
		      ruleOutputs[combinations[0][l].ruleId][combinations[0][l].tokenId];

		  putOut (&outs, ambigOut,
			  combinations[0][l].tokenId + combinations[0][l].patNum - 1,
			  spaces);
		}

	      j += ambigMap[j].maxPat;
	    }
	  // make it else if nodes.size()==1
	  else
	    {
	      putCombination (&combNodes, nodes);
	      // put the method above this method
	      putOut (&outs, ruleOutputs[nodes[0].ruleId][nodes[0].tokenId],
		      nodes[0].tokenId + nodes[0].patNum - 1, spaces);
	      j += nodes[0].patNum;
	    }
	}

      // put only different outputs
      for (unsigned j = 0; j < outs.size (); j++)
	{
	  if ((!ambigInfo.empty () && ambigInfo[i].combinations.size () > 1)
	      || find (finalOuts->begin (), finalOuts->end (), outs[j])
		  == finalOuts->end ())
	    {
	      finalOuts->push_back (outs[j]);
	      finalCombNodes->push_back (combNodes[j]);
	    }
	}
    }
}

void
putCombination (vector<vector<ambiguous_transfer::Node> >* combinations,
		vector<ambiguous_transfer::Node> combination)
{
  for (unsigned i = 0; i < combinations->size (); i++)
    (*combinations)[i].insert ((*combinations)[i].end (), combination.begin (),
			       combination.end ());
}

vector<vector<ambiguous_transfer::Node> >
putCombinations (vector<vector<ambiguous_transfer::Node> > combinations,
		 vector<vector<ambiguous_transfer::Node> > nestedcombinations)
{

  vector<vector<ambiguous_transfer::Node> > newcombinations;

  for (unsigned i = 0; i < combinations.size (); i++)
    {
      for (unsigned j = 0; j < nestedcombinations.size (); j++)
	{
	  vector<ambiguous_transfer::Node> newcombination = vector<
	      ambiguous_transfer::Node> (combinations[i]);
	  // +1 to skip dummy node
	  newcombination.insert (newcombination.end (),
				 nestedcombinations[j].begin () + 1,
				 nestedcombinations[j].end ());

	  newcombinations.push_back (newcombination);
	}
    }

  return newcombinations;
}

void
putOut (vector<string>* outputs, string output, unsigned tokenIndex,
	vector<string> spaces)
{

  for (unsigned i = 0; i < outputs->size (); i++)
    {
      (*outputs)[i] += output;
      (*outputs)[i] += (spaces[tokenIndex]);
    }
}

vector<string>
putOuts (vector<string> outputs, vector<string> nestedOutputs)
{
  vector<string> newOutputs;

  for (unsigned i = 0; i < outputs.size (); i++)
    {
      for (unsigned j = 0; j < nestedOutputs.size (); j++)
	{
	  string newOutput;

	  newOutput += outputs[i];
	  newOutput += nestedOutputs[j];

	  newOutputs.push_back (newOutput);
	}
    }

  return newOutputs;
}

void
getCombinations (ambiguous_transfer::Node root, vector<ambiguous_transfer::Node> path,
		 vector<vector<ambiguous_transfer::Node> >* ambigRules)
{

  path.push_back (root);

  for (unsigned i = 0; i < root.neighbors.size (); i++)
    getCombinations (root.neighbors[i], path, ambigRules);

  if (root.neighbors.empty ())
    {
      // if the rule0 in a combi   nation , don't count it
      for (unsigned i = 0; i < path.size (); i++)
	if (path[i].ruleId == 0)
	  return;

      ambigRules->push_back (path);
    }
}

void
ambiguous_transfer::normaliseWeights (vector<float>* weights,
				      vector<ambiguous_transfer::AmbigInfo> ambigInfo)
{

  unsigned weigInd = 0;

  for (unsigned j = 0; j < ambigInfo.size (); j++)
    {
      // get sum of weights of an ambigInfo
      float sum = 0;
      for (unsigned k = 0; k < ambigInfo[j].combinations.size (); k++)
	{
	  sum += (*weights)[weigInd + k];
	}
      // Then normalize it
      for (unsigned k = 0; k < ambigInfo[j].combinations.size (); k++)
	{
	  // if sum=0 , to avoid nans we will make them all equal in weights
	  if (sum)
	    (*weights)[weigInd + k] /= sum;
	  else
	    (*weights)[weigInd + k] = 1 / ambigInfo[j].combinations.size ();
	}

      // update weighInd
      weigInd += ambigInfo[j].combinations.size ();
    }
}

void
getMaxPat (int curMaxPat, unsigned curToken,
	   map<unsigned, vector<pair<unsigned, unsigned> > > tokenRules, unsigned* count)
{
  if (curMaxPat == 0)
    return;

  int maxPat = 0;
  vector<pair<unsigned, unsigned> > rules = tokenRules[curToken];

  if (rules.size ())
    maxPat = rules[0].second;

  (*count)++;
  getMaxPat (max (curMaxPat - 1, maxPat - curMaxPat), curToken + 1, tokenRules, count);
}

void
ambiguous_transfer::getAmbigInfo (
    map<unsigned, vector<pair<unsigned, unsigned> > > tokenRules,
    map<unsigned, vector<ambiguous_transfer::Node> > nodesPool,
    vector<ambiguous_transfer::AmbigInfo>* ambigInfo, unsigned* combNum)
{
  *combNum = 0;
  for (unsigned tokId = 0; tokId < tokenRules.size ();)
    {
      unsigned maxPat = 0;
      vector<pair<unsigned, unsigned> > rules = tokenRules[tokId];
      getMaxPat (rules[0].second, tokId, tokenRules, &maxPat);

      // if there is ambiguity
      if (nodesPool[tokId].size () > 1)
	{
	  ambiguous_transfer::AmbigInfo ambig = ambiguous_transfer::AmbigInfo (tokId,
									       maxPat);

	  ambiguous_transfer::Node dummy = ambiguousGraph (tokenRules, nodesPool, tokId,
							   maxPat);
	  getCombinations (dummy, vector<ambiguous_transfer::Node> (),
			   &ambig.combinations);

	  if (!ambig.combinations.empty ())
	    ambigInfo->push_back (ambig);

	  *combNum += ambig.combinations.size ();
	}
      tokId += maxPat;
    }
}

map<unsigned, vector<ambiguous_transfer::Node> >
ambiguous_transfer::getNodesPool (
    map<unsigned, vector<pair<unsigned, unsigned> > > tokenRules)
{
  map<unsigned, vector<ambiguous_transfer::Node> > nodesPool;
  for (map<unsigned, vector<pair<unsigned, unsigned> > >::iterator it =
      tokenRules.begin (); it != tokenRules.end (); it++)
    {
      unsigned tokenId = it->first;
      vector<pair<unsigned, unsigned> > rules = it->second;
      for (unsigned i = 0; i < rules.size (); i++)
	{
	  unsigned ruleId = rules[i].first;
	  unsigned patNum = rules[i].second;
	  ambiguous_transfer::Node node = ambiguous_transfer::Node (tokenId, ruleId,
								    patNum);
	  nodesPool[tokenId].push_back (node);
	}
    }
  return nodesPool;
}

// to sort rules in tokenRules descendingly by their number of pattern items
bool
sortParameter (pair<unsigned, unsigned> a, pair<unsigned, unsigned> b)
{
  return (a.second > b.second);
}

string
noRuleOut (vector<string> analysis)
{
  vector<string> out;
  // unknown word
  if (analysis[0][0] == '*')
    out.push_back ("^unknown<unknown>{^");
  else
    out.push_back ("^default<default>{^");
  out.insert (out.end (), analysis.begin (), analysis.end ());
  out.push_back ("$}$");

  string str;
  for (unsigned i = 0; i < out.size (); i++)
    str += out[i];

  return str;
}

void
nestedRules (vector<string> tlTokens, string output,
	     vector<pair<unsigned, unsigned> > curNestedRules, vector<string>* outputs,
	     vector<vector<pair<unsigned, unsigned> > >* nestedOutsRules,
	     map<unsigned, map<unsigned, string> > ruleOuts,
	     map<unsigned, vector<pair<unsigned, unsigned> > > tokenRules,
	     vector<string> spaces, unsigned curPatNum, unsigned curTokIndex)
{
  vector<pair<unsigned, unsigned> > rulesApplied = tokenRules[curTokIndex];

  for (unsigned i = 0; i < rulesApplied.size (); i++)
    {
      unsigned rule = rulesApplied[i].first;
      unsigned patNum = rulesApplied[i].second;

      if (patNum <= curPatNum)
	{
	  vector<pair<unsigned, unsigned> > newCurNestedRules = vector<
	      pair<unsigned, unsigned> > (curNestedRules);
	  newCurNestedRules.push_back (pair<unsigned, unsigned> (rule, curTokIndex));

	  string newOutput = output;

	  string ruleOut = ruleOuts[rule][curTokIndex];
	  ruleOut += spaces[curTokIndex + patNum - 1];
	  newOutput += ruleOut;

	  // remove that rule from all tokens

	  tokenRules[curTokIndex].erase (tokenRules[curTokIndex].begin ());

	  if (curPatNum == patNum)
	    {
	      (*outputs).push_back (newOutput);
	      (*nestedOutsRules).push_back (newCurNestedRules);
	    }
	  else
	    {
	      nestedRules (tlTokens, output, newCurNestedRules, outputs, nestedOutsRules,
			   ruleOuts, tokenRules, spaces, curPatNum - patNum,
			   curTokIndex + patNum);
	    }
	}
    }
}

void
pushDistinct (map<unsigned, vector<pair<unsigned, unsigned> > >* tokenRules,
	      unsigned tlTokInd, xmlNode* rule, unsigned patNum)
{
  vector<pair<unsigned, unsigned> > pairs = (*tokenRules)[tlTokInd];
//  cout << "here001" << endl;
//  cout << "here002" << endl;
  for (unsigned i = 0; i < pairs.size (); i++)
    {
      if (pairs[i].first == xml_operations::xml_operations::getAttValUnsg (rule, ID))
	return;
    }
//  xml_operations::xml_operations::getAttValUnsg (rule, ID);
//  cout << "here003" << endl;
  (*tokenRules)[tlTokInd].push_back (
      pair<unsigned, unsigned> (xml_operations::xml_operations::getAttValUnsg (rule, ID),
				patNum));
//  cout << "here004" << endl;
  sort ((*tokenRules)[tlTokInd].begin (), (*tokenRules)[tlTokInd].end (), sortParameter);
}

void
printNodeAttrs (xmlNode* node)
{
//	cout << node.name() << endl;
//	for (xmlNode*::attribute_iterator it = node.attributes_begin ();
//			it != node.attributes_end();
//			it++
//			)
//			{
//				cout << it->name () << "=" << it->value () << "; ";
//			}
//			cout << endl << endl;
}

void
ruleOuts (map<unsigned, map<unsigned, string> >* ruleOuts,
	  map<unsigned, vector<pair<unsigned, unsigned> > >* tokenRules,
	  vector<string> slTokens, vector<vector<string> > slTags,
	  vector<string> tlTokens, vector<vector<string> > tlTags,
	  map<xmlNode*, vector<pair<unsigned, unsigned> > > rulesApplied,
	  map<string, vector<vector<string> > > attrs, map<string, vector<string> > lists,
	  map<string, string>* vars, vector<string> spaces, string localeId)
{
  //cout << "Inside  " << "ruleOuts" << endl;

  for (map<xmlNode*, vector<pair<unsigned, unsigned> > >::iterator it =
      rulesApplied.begin (); it != rulesApplied.end (); ++it)
    {
      xmlNode* rule = it->first;
      for (unsigned i = 0; i < rulesApplied[rule].size (); i++)
	{
	  vector<vector<string> > slAnalysisTokens, tlAnalysisTokens;

	  // format tokens and their tags into analysisTokens

	  unsigned firstMatTok = rulesApplied[rule][i].first;
	  unsigned patNum = rulesApplied[rule][i].second;

	  for (unsigned tokInd = firstMatTok; tokInd < firstMatTok + patNum; tokInd++)
	    {
	      vector<string> slAnalysisToken = formatTokenTags (slTokens[tokInd],
								slTags[tokInd]);

	      slAnalysisTokens.push_back (slAnalysisToken);

	      vector<string> tlAnalysisToken = formatTokenTags (tlTokens[tokInd],
								tlTags[tokInd]);

	      tlAnalysisTokens.push_back (tlAnalysisToken);
	    }

	  // insert the rule (if not found) then sort the vector
	  pushDistinct (tokenRules, firstMatTok, rule, patNum);

	  vector<string> output;
	  if (xml_operations::xml_operations::getAttValUnsg (rule, ID) == 0)
	    output.push_back (noRuleOut (tlAnalysisTokens[0]));
	  else
	    output = ruleExe (rule, &slAnalysisTokens, &tlAnalysisTokens, attrs, lists,
			      vars, spaces, firstMatTok, localeId); // first pattern index

	  string str;
	  for (unsigned j = 0; j < output.size (); j++)
	    str += output[j];

	  (*ruleOuts)[xml_operations::xml_operations::getAttValUnsg (rule, ID)][firstMatTok] =
	      str;
	}
    }

}

void
ambiguous_transfer::ruleOuts (
    map<unsigned, map<unsigned, string> >* ruleOuts,
    map<unsigned, vector<pair<unsigned, unsigned> > >* tokenRules,
    vector<string> slTokens, vector<vector<string> > slTags, vector<string> tlTokens,
    vector<vector<string> > tlTags,
    map<xmlNode*, vector<pair<unsigned, unsigned> > > rulesApplied,
    map<string, vector<vector<string> > > attrs, map<string, vector<string> > lists,
    map<string, string>* vars, vector<string> spaces, string localeId)
{
//  cout << "Inside  " << "ruleOuts" << endl;

  for (map<xmlNode*, vector<pair<unsigned, unsigned> > >::iterator it =
      rulesApplied.begin (); it != rulesApplied.end (); ++it)
    {
      xmlNode* rule = it->first;
      for (unsigned i = 0; i < rulesApplied[rule].size (); i++)
	{
	  vector<vector<string> > slAnalysisTokens, tlAnalysisTokens;

	  // format tokens and their tags into analysisTokens
//	  cout << "here01" << endl;
	  unsigned firstMatTok = rulesApplied[rule][i].first;
	  unsigned patNum = rulesApplied[rule][i].second;

	  for (unsigned tokInd = firstMatTok; tokInd < firstMatTok + patNum; tokInd++)
	    {
	      vector<string> slAnalysisToken = formatTokenTags (slTokens[tokInd],
								slTags[tokInd]);

	      slAnalysisTokens.push_back (slAnalysisToken);

	      vector<string> tlAnalysisToken = formatTokenTags (tlTokens[tokInd],
								tlTags[tokInd]);

	      tlAnalysisTokens.push_back (tlAnalysisToken);

	    }
//	  cout << "here02" << endl;
	  // insert the rule (if not found) then sort the vector
	  pushDistinct (tokenRules, firstMatTok, rule, patNum);
//	  cout << "here03" << endl;
	  vector<string> output;

	  cout << "before" << endl;
	  for (unsigned j = 0; j < tlAnalysisTokens.size (); j++)
	    {
	      for (unsigned k = 0; k < tlAnalysisTokens[j].size (); k++)
		{
		  cout << tlAnalysisTokens[j][k] << "  ";
		}
	      cout << endl;
	    }

	  if (xml_operations::getAttValUnsg (rule, ID) == 0)
	    output.push_back (noRuleOut (tlAnalysisTokens[0]));
	  else
	    output = ruleExe (rule, &slAnalysisTokens, &tlAnalysisTokens, attrs, lists,
			      vars, spaces, firstMatTok, localeId); // first pattern index

	  cout << "after" << endl;
	  for (unsigned j = 0; j < tlAnalysisTokens.size (); j++)
	    {
	      for (unsigned k = 0; k < tlAnalysisTokens[j].size (); k++)
		{
		  cout << tlAnalysisTokens[j][k] << "  ";
		}
	      cout << endl;
	    }

	  //	  cout << "here04" << endl;
	  string str;
	  for (unsigned j = 0; j < output.size (); j++)
	    str += output[j];

	  (*ruleOuts)[xml_operations::getAttValUnsg (rule, ID)][firstMatTok] = str;
	}
    }

}

vector<string>
ruleExe (xmlNode* rule, vector<vector<string> >* slAnalysisTokens,
	 vector<vector<string> >* tlAnalysisTokens,
	 map<string, vector<vector<string> > > attrs, map<string, vector<string> > lists,
	 map<string, string>* vars, vector<string> spaces, unsigned firPat,
	 string localeId)
{

  printNodeAttrs (rule);

  vector<string> output;

  map<unsigned, unsigned> paramToPattern = map<unsigned, unsigned> ();

  xmlNode* action = xml_operations::getChild (rule, ACTION);

  for (xmlNode* child = xml_operations::getFirstChild (action); child; child =
      xml_operations::getFirstNext (child))
    {
      vector<string> result;

      string childName = xml_operations::getName (child);
      if (childName == LET)
	{
	  let (child, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces, firPat,
	       localeId, paramToPattern);
	}
      else if (childName == CHOOSE)
	{
	  result = choose (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
			   spaces, firPat, localeId, paramToPattern);
	}
      else if (childName == CALL_MACRO)
	{
	  result = callMacro (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists,
			      vars, spaces, firPat, localeId, paramToPattern);
	}
      else if (childName == OUT)
	{
	  result = out (child, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces,
			firPat, localeId, paramToPattern);
	}

      else if (childName == MODIFY_CASE)
	modifyCase (child, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces,
		    firPat, localeId, paramToPattern);

      else if (childName == APPEND)
	append (child, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces, firPat,
		localeId, paramToPattern);

      output.insert (output.end (), result.begin (), result.end ());
    }

  return output;
}

vector<string>
out (xmlNode* out, vector<vector<string> >* slAnalysisTokens,
     vector<vector<string> >* tlAnalysisTokens,
     map<string, vector<vector<string> > > attrs, map<string, string>* vars,
     vector<string> spaces, unsigned firPat, string localeId,
     map<unsigned, unsigned> paramToPattern)
{

  printNodeAttrs (out);

  vector<string> output;

  for (xmlNode* child = xml_operations::getFirstChild (out); child; child =
      xml_operations::getFirstNext (child))
    {

      vector<string> result;

      string childName = xml_operations::getName (child);
      if (childName == B)
	{
	  result.push_back (b (child, spaces, firPat, localeId, paramToPattern));
	}
      else if (childName == CHUNK)
	{
	  result = chunk (child, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces,
			  firPat, localeId, paramToPattern);
	}

      output.insert (output.end (), result.begin (), result.end ());
    }

  return output;
}

vector<string>
chunk (xmlNode* chunkNode, vector<vector<string> >* slAnalysisTokens,
       vector<vector<string> >* tlAnalysisTokens,
       map<string, vector<vector<string> > > attrs, map<string, string>* vars,
       vector<string> spaces, unsigned firPat, string localeId,
       map<unsigned, unsigned> paramToPattern)
{

  printNodeAttrs (chunkNode);

  vector<string> output;

  output.push_back ("^");

  string name = xml_operations::getAttVal (chunkNode, NAME);
  if (name.empty ())
    {
      string varName = xml_operations::getAttVal (chunkNode, NAME_FROM);
      name = (*vars)[varName];
    }

  output.push_back (name);

  // tags element must be first item
  vector<vector<string> > tagsResult;
  xmlNode* tags = xml_operations::getChild (chunkNode, TAGS);
  for (xmlNode* tag = xml_operations::getFirstChild (tags); tag; tag =
      xml_operations::getFirstNext (tag))
    {

      printNodeAttrs (tag);

      vector<string> result;

      xmlNode* child = xml_operations::getFirstChild (tag);
      string childName = xml_operations::getName (child);

      if (childName == CLIP)
	{
	  result = clip (child, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces,
			 firPat, localeId, paramToPattern);
	}
      else if (childName == CONCAT)
	{
	  result = concat (child, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces,
			   firPat, localeId, paramToPattern);
	}
      else if (childName == LIT_TAG)
	{
	  result = litTag (child);
	}
      else if (childName == LIT)
	{
	  result.push_back (lit (child));
	}
      else if (childName == B)
	{
	  result.push_back (b (child, spaces, firPat, localeId, paramToPattern));
	}
      else if (childName == CASE_OF)
	{
	  result.push_back (
	      caseOf (child, slAnalysisTokens, tlAnalysisTokens, localeId,
		      paramToPattern));
	}
      else if (childName == GET_CASE_FROM)
	{
	  result.push_back (
	      getCaseFrom (child, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces,
			   firPat, localeId, paramToPattern));
	}
      else if (childName == VAR)
	{
	  result.push_back (var (child, vars));
	}

      tagsResult.push_back (result);
      output.insert (output.end (), result.begin (), result.end ());
    }

  output.push_back ("{");

  for (xmlNode* child = xml_operations::getFirstNext (tags); child; child =
      xml_operations::getFirstNext (child))
    {

      //      printNodeAttrs (child);
      string childName = xml_operations::getName (child);
      if (childName == LU)
	{
	  // lu is the same as concat
	  vector<string> result = concat (child, slAnalysisTokens, tlAnalysisTokens,
					  attrs, vars, spaces, firPat, localeId,
					  paramToPattern, tagsResult);
	  output.push_back ("^");
	  output.insert (output.end (), result.begin (), result.end ());
	  output.push_back ("$");
	}
      else if (childName == MLU)
	{
	  output.push_back ("^");
	  // has only lu children
	  for (xmlNode* lu = xml_operations::getFirstChild (child); lu; lu =
	      xml_operations::getFirstNext (lu))
	    {
	      vector<string> result = concat (lu, slAnalysisTokens, tlAnalysisTokens,
					      attrs, vars, spaces, firPat, localeId,
					      paramToPattern, tagsResult);
	      output.insert (output.end (), result.begin (), result.end ());
	      if (xml_operations::getFirstNext (lu))
		output.push_back ("+");
	    }
	  output.push_back ("$");
	}
      else if (childName == B)
	{
	  output.push_back (b (child, spaces, firPat, localeId, paramToPattern));
	}
    }

  output.push_back ("}$");

  return output;
}

vector<string>
callMacro (xmlNode* callMacroNode, vector<vector<string> >* slAnalysisTokens,
	   vector<vector<string> >* tlAnalysisTokens,
	   map<string, vector<vector<string> > > attrs,
	   map<string, vector<string> > lists, map<string, string>* vars,
	   vector<string> spaces, unsigned firPat, string localeId,
	   map<unsigned, unsigned> paramToPattern)
{

  printNodeAttrs (callMacroNode);

  vector<string> output;

  string macroName = xml_operations::getAttVal (callMacroNode, N);

  map<unsigned, unsigned> newParamToPattern;
  unsigned i = 1;
  for (xmlNode* with_param = xml_operations::getFirstChild (callMacroNode); with_param;
      with_param = xml_operations::getFirstNext (with_param))
    {
      unsigned pos = xml_operations::xml_operations::getAttValUnsg (with_param, POS);
      if (paramToPattern.size ())
	pos = paramToPattern[pos];

      newParamToPattern[i++] = pos;
    }

  xmlNode* transfer = xml_operations::getRoot (callMacroNode);

  xmlNode* macros = xml_operations::getChild (transfer, SECTION_DEF_MACROS);

  xmlNode* macro;
  for (macro = xml_operations::getFirstChild (macros); macro; macro =
      xml_operations::getFirstNext (macro))
    {
      if (xml_operations::getAttVal (macro, N) == macroName)
	break;
    }

  for (xmlNode* child = xml_operations::getFirstChild (macro); child; child =
      xml_operations::getFirstNext (child))
    {
      vector<string> result;

      string childName = xml_operations::getName (child);
      if (childName == CHOOSE)
	result = choose (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
			 spaces, firPat, localeId, newParamToPattern);

      else if (childName == OUT)
	result = out (child, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces,
		      firPat, localeId, newParamToPattern);

      else if (childName == CALL_MACRO)
	result = callMacro (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
			    spaces, firPat, localeId, newParamToPattern);

      else if (childName == LET)
	let (child, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces, firPat,
	     localeId, newParamToPattern);

      else if (childName == MODIFY_CASE)
	modifyCase (child, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces,
		    firPat, localeId, newParamToPattern);

      else if (childName == APPEND)
	append (child, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces, firPat,
		localeId, newParamToPattern);

      output.insert (output.end (), result.begin (), result.end ());
    }

  return output;
}

vector<string>
findAttrPart (vector<string> tokenTags, vector<vector<string> > attrTags)
{

  vector<string> matchedTags;
  for (unsigned i = 0; i < tokenTags.size (); i++)
    {
      for (unsigned j = 0; j < attrTags.size (); j++)
	{
	  if (tokenTags[i] == ("<" + attrTags[j][0] + ">"))
	    {
	      matchedTags.push_back (tokenTags[i]);
	      for (unsigned k = 1; k < attrTags[j].size () && (k + i) < tokenTags.size ();
		  k++)
		{

		  if (tokenTags[i + k] == ("<" + attrTags[j][k] + ">"))
		    matchedTags.push_back (tokenTags[i + k]);
		  else
		    break;
		}
	      if (matchedTags.size () == attrTags[j].size ())
		return matchedTags;
	      else
		matchedTags.clear ();
	    }
	}
    }
  return matchedTags;
}

bool
equal (xmlNode* equal, vector<vector<string> >* slAnalysisTokens,
       vector<vector<string> >* tlAnalysisTokens,
       map<string, vector<vector<string> > > attrs, map<string, string>* vars,
       vector<string> spaces, unsigned firPat, string localeId,
       map<unsigned, unsigned> paramToPattern)
{

  printNodeAttrs (equal);

  xmlNode* firstChild = xml_operations::getFirstChild (equal);
  vector<string> firstResult;

  string firstName = xml_operations::getName (firstChild);
  if (firstName == CLIP)
    {
      firstResult = clip (firstChild, slAnalysisTokens, tlAnalysisTokens, attrs, vars,
			  spaces, firPat, localeId, paramToPattern);
    }
  else if (firstName == CONCAT)
    {
      firstResult = concat (firstChild, slAnalysisTokens, tlAnalysisTokens, attrs, vars,
			    spaces, firPat, localeId, paramToPattern);
    }
  else if (firstName == LIT_TAG)
    {
      firstResult = litTag (firstChild);
    }
  else if (firstName == LIT)
    {
      firstResult.push_back (lit (firstChild));
    }
  else if (firstName == B)
    {
      firstResult.push_back (b (firstChild, spaces, firPat, localeId, paramToPattern));
    }
  else if (firstName == CASE_OF)
    {
      firstResult.push_back (
	  caseOf (firstChild, slAnalysisTokens, tlAnalysisTokens, localeId,
		  paramToPattern));
    }
  else if (firstName == GET_CASE_FROM)
    {
      firstResult.push_back (
	  getCaseFrom (firstChild, slAnalysisTokens, tlAnalysisTokens, attrs, vars,
		       spaces, firPat, localeId, paramToPattern));
    }
  else if (firstName == VAR)
    {
      firstResult.push_back (var (firstChild, vars));
    }

  xmlNode* secondChild = xml_operations::getFirstNext (firstChild);
  vector<string> secondResult;

  string secondName = xml_operations::getName (secondChild);
  if (secondName == CLIP)
    {
      secondResult = clip (secondChild, slAnalysisTokens, tlAnalysisTokens, attrs, vars,
			   spaces, firPat, localeId, paramToPattern);
    }
  else if (secondName == CONCAT)
    {
      secondResult = concat (secondChild, slAnalysisTokens, tlAnalysisTokens, attrs, vars,
			     spaces, firPat, localeId, paramToPattern);
    }
  else if (secondName == LIT_TAG)
    {
      secondResult = litTag (secondChild);
    }
  else if (secondName == LIT)
    {
      secondResult.push_back (lit (secondChild));
    }
  else if (secondName == B)
    {
      secondResult.push_back (b (secondChild, spaces, firPat, localeId, paramToPattern));
    }
  else if (secondName == CASE_OF)
    {
      secondResult.push_back (
	  caseOf (secondChild, slAnalysisTokens, tlAnalysisTokens, localeId,
		  paramToPattern));
    }
  else if (secondName == GET_CASE_FROM)
    {
      secondResult.push_back (
	  getCaseFrom (secondChild, slAnalysisTokens, tlAnalysisTokens, attrs, vars,
		       spaces, firPat, localeId, paramToPattern));
    }
  else if (secondName == VAR)
    {
      secondResult.push_back (var (secondChild, vars));
    }

  string firstStr, secondStr;
  for (unsigned i = 0; i < firstResult.size (); i++)
    {
      firstStr += firstResult[i];
    }
  for (unsigned i = 0; i < secondResult.size (); i++)
    {
      secondStr += secondResult[i];
    }

//  cout << "firstStr=" << firstStr << " , secondStr=" << secondStr << endl;

  if (xml_operations::getAttVal (equal, CASE_LESS) == "yes")
    {
      return !(CLExec::compareCaseless (firstStr, secondStr, localeId));
    }
  else
    {
      return !(CLExec::compare (firstStr, secondStr));
    }
}

vector<string>
choose (xmlNode* chooseNode, vector<vector<string> >* slAnalysisTokens,
	vector<vector<string> >* tlAnalysisTokens,
	map<string, vector<vector<string> > > attrs, map<string, vector<string> > lists,
	map<string, string>* vars, vector<string> spaces, unsigned firPat,
	string localeId, map<unsigned, unsigned> paramToPattern)
{
  //cout << "Inside  " << "choose" << endl;
  printNodeAttrs (chooseNode);

  vector<string> output;

  for (xmlNode* child = xml_operations::getFirstChild (chooseNode); child; child =
      xml_operations::getFirstNext (child))
    {
      bool condition = false;

      string childName = xml_operations::getName (child);
      if (childName == WHEN)
	{
	  xmlNode* testNode = xml_operations::getChild (child, TEST);

	  condition = test (testNode, slAnalysisTokens, tlAnalysisTokens, attrs, lists,
			    vars, spaces, firPat, localeId, paramToPattern);
	}
      else
	{
	  // otherwise
	  condition = true;
	}

//      cout << "condition=" << condition << endl;
      if (condition)
	{
	  for (xmlNode* inst = xml_operations::getFirstChild (child); inst; inst =
	      xml_operations::getFirstNext (inst))
	    {
	      vector<string> result;

	      string instName = xml_operations::getName (inst);
	      if (instName == CHOOSE)
		result = choose (inst, slAnalysisTokens, tlAnalysisTokens, attrs, lists,
				 vars, spaces, firPat, localeId, paramToPattern);

	      else if (instName == OUT)
		result = out (inst, slAnalysisTokens, tlAnalysisTokens, attrs, vars,
			      spaces, firPat, localeId, paramToPattern);

	      else if (instName == CALL_MACRO)
		result = callMacro (inst, slAnalysisTokens, tlAnalysisTokens, attrs,
				    lists, vars, spaces, firPat, localeId,
				    paramToPattern);

	      else if (instName == LET)
		let (inst, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces,
		     firPat, localeId, paramToPattern);

	      else if (instName == MODIFY_CASE)
		modifyCase (inst, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces,
			    firPat, localeId, paramToPattern);

	      else if (instName == APPEND)
		append (inst, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces,
			firPat, localeId, paramToPattern);

	      output.insert (output.end (), result.begin (), result.end ());
	    }
	  break;
	}
    }

  return output;
}

bool
test (xmlNode* test, vector<vector<string> >* slAnalysisTokens,
      vector<vector<string> >* tlAnalysisTokens,
      map<string, vector<vector<string> > > attrs, map<string, vector<string> > lists,
      map<string, string>* vars, vector<string> spaces, unsigned firPat, string localeId,
      map<unsigned, unsigned> paramToPattern)
{

  printNodeAttrs (test);

  xmlNode* child = xml_operations::getFirstChild (test);
  string childName = xml_operations::getName (child);

  bool condition = false;

  if (childName == EQUAL)
    {
      condition = equal (child, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces,
			 firPat, localeId, paramToPattern);
    }
  else if (childName == AND)
    {
      condition = And (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
		       spaces, firPat, localeId, paramToPattern);
    }
  else if (childName == OR)
    {
      condition = Or (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
		      spaces, firPat, localeId, paramToPattern);
    }
  else if (childName == NOT)
    {
      condition = Not (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
		       spaces, firPat, localeId, paramToPattern);
    }
  else if (childName == IN)
    {
      condition = in (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
		      spaces, firPat, localeId, paramToPattern);
    }

  return condition;
}

bool
And (xmlNode* andNode, vector<vector<string> >* slAnalysisTokens,
     vector<vector<string> >* tlAnalysisTokens,
     map<string, vector<vector<string> > > attrs, map<string, vector<string> > lists,
     map<string, string>* vars, vector<string> spaces, unsigned firPat, string localeId,
     map<unsigned, unsigned> paramToPattern)
{

  printNodeAttrs (andNode);

  bool condition = false;

  for (xmlNode* child = xml_operations::getFirstChild (andNode); child; child =
      xml_operations::getFirstNext (child))
    {
      string childName = xml_operations::getName (child);
      if (childName == EQUAL)
	{
	  condition = equal (child, slAnalysisTokens, tlAnalysisTokens, attrs, vars,
			     spaces, firPat, localeId, paramToPattern);
	}
      else if (childName == AND)
	{
	  condition = And (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
			   spaces, firPat, localeId, paramToPattern);
	}
      else if (childName == OR)
	{
	  condition = Or (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
			  spaces, firPat, localeId, paramToPattern);
	}
      else if (childName == NOT)
	{
	  condition = Not (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
			   spaces, firPat, localeId, paramToPattern);
	}
      else if (childName == IN)
	{
	  condition = in (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
			  spaces, firPat, localeId, paramToPattern);
	}

      if (!condition)
	break;
    }

  return condition;
}

bool
Or (xmlNode* orNode, vector<vector<string> >* slAnalysisTokens,
    vector<vector<string> >* tlAnalysisTokens,
    map<string, vector<vector<string> > > attrs, map<string, vector<string> > lists,
    map<string, string>* vars, vector<string> spaces, unsigned firPat, string localeId,
    map<unsigned, unsigned> paramToPattern)
{

  printNodeAttrs (orNode);

  bool condition = false;

  for (xmlNode* child = xml_operations::getFirstChild (orNode); child; child =
      xml_operations::getFirstNext (child))
    {
      string childName = xml_operations::getName (child);
      if (childName == EQUAL)
	{
	  condition = equal (child, slAnalysisTokens, tlAnalysisTokens, attrs, vars,
			     spaces, firPat, localeId, paramToPattern);
	}
      else if (childName == AND)
	{
	  condition = And (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
			   spaces, firPat, localeId, paramToPattern);
	}
      else if (childName == OR)
	{
	  condition = Or (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
			  spaces, firPat, localeId, paramToPattern);
	}
      else if (childName == NOT)
	{
	  condition = Not (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
			   spaces, firPat, localeId, paramToPattern);
	}
      else if (childName == IN)
	{
	  condition = in (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
			  spaces, firPat, localeId, paramToPattern);
	}

      if (condition)
	break;
    }

  return condition;
}

bool
in (xmlNode* inNode, vector<vector<string> >* slAnalysisTokens,
    vector<vector<string> >* tlAnalysisTokens,
    map<string, vector<vector<string> > > attrs, map<string, vector<string> > lists,
    map<string, string>* vars, vector<string> spaces, unsigned firPat, string localeId,
    map<unsigned, unsigned> paramToPattern)
{

  printNodeAttrs (inNode);

  xmlNode* firstChild = xml_operations::getFirstChild (inNode);
  vector<string> firstResult;

  string firstName = xml_operations::getName (firstChild);
  if (firstName == CLIP)
    {
      firstResult = clip (firstChild, slAnalysisTokens, tlAnalysisTokens, attrs, vars,
			  spaces, firPat, localeId, paramToPattern);
    }
  else if (firstName == CONCAT)
    {
      firstResult = concat (firstChild, slAnalysisTokens, tlAnalysisTokens, attrs, vars,
			    spaces, firPat, localeId, paramToPattern);
    }
  else if (firstName == LIT_TAG)
    {
      firstResult = litTag (firstChild);
    }
  else if (firstName == LIT)
    {
      firstResult.push_back (lit (firstChild));
    }
  else if (firstName == B)
    {
      firstResult.push_back (b (firstChild, spaces, firPat, localeId, paramToPattern));
    }
  else if (firstName == CASE_OF)
    {
      firstResult.push_back (
	  caseOf (firstChild, slAnalysisTokens, tlAnalysisTokens, localeId,
		  paramToPattern));
    }
  else if (firstName == GET_CASE_FROM)
    {
      firstResult.push_back (
	  getCaseFrom (firstChild, slAnalysisTokens, tlAnalysisTokens, attrs, vars,
		       spaces, firPat, localeId, paramToPattern));
    }
  else if (firstName == VAR)
    {
      firstResult.push_back (var (firstChild, vars));
    }

  string firstStr;
  for (unsigned i = 0; i < firstResult.size (); i++)
    {
      firstStr += firstResult[i];
    }

  xmlNode* listNode = xml_operations::getFirstNext (firstChild);

  string listName = xml_operations::getAttVal (listNode, N);
  vector<string> list = lists[listName];

  if (xml_operations::getAttVal (inNode, CASE_LESS) == "yes")
    {
      for (unsigned i = 0; i < list.size (); i++)
	{
	  if (!CLExec::compareCaseless (firstStr, list[i], localeId))
	    return true;
	}
    }
  else
    {
      for (unsigned i = 0; i < list.size (); i++)
	{
	  if (!CLExec::compare (firstStr, list[i]))
	    return true;
	}
    }

  return false;
}

bool
Not (xmlNode* NotNode, vector<vector<string> >* slAnalysisTokens,
     vector<vector<string> >* tlAnalysisTokens,
     map<string, vector<vector<string> > > attrs, map<string, vector<string> > lists,
     map<string, string>* vars, vector<string> spaces, unsigned firPat, string localeId,
     map<unsigned, unsigned> paramToPattern)
{

  printNodeAttrs (NotNode);

  xmlNode* child = xml_operations::getFirstChild (NotNode);
  string childName = xml_operations::getName (child);

  bool condition = false;

  if (childName == EQUAL)
    {
      condition = equal (child, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces,
			 firPat, localeId, paramToPattern);
    }
  else if (childName == AND)
    {
      condition = And (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
		       spaces, firPat, localeId, paramToPattern);
    }
  else if (childName == OR)
    {
      condition = Or (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
		      spaces, firPat, localeId, paramToPattern);
    }
  else if (childName == NOT)
    {
      condition = Not (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
		       spaces, firPat, localeId, paramToPattern);
    }
  else if (childName == IN)
    {
      condition = in (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
		      spaces, firPat, localeId, paramToPattern);
    }

  return !condition;
}

vector<string>
litTag (xmlNode* litTag)
{

  printNodeAttrs (litTag);

  // splitting tags by '.'
  string tagsString = xml_operations::getAttVal (litTag, V);
  char tagsChars[tagsString.size ()];
  strcpy (tagsChars, tagsString.c_str ());

  vector<string> tags;

  char * tag;
  tag = strtok (tagsChars, ".");
  while (tag != NULL)
    {
      tags.push_back ("<" + string (tag) + ">");
      tag = strtok (NULL, ".");
    }

  return tags;
}

string
lit (xmlNode* lit)
{
  printNodeAttrs (lit);

  string litValue = xml_operations::getAttVal (lit, V);
  return litValue;
}

string
var (xmlNode* var, map<string, string>* vars)
{
  printNodeAttrs (var);

  string varName = xml_operations::getAttVal (var, N);
  string varValue = (*vars)[varName];
//  cout << "varname=" << varName << " , value=" << (*vars)[varName] << endl;
  return varValue;
}

void
let (xmlNode* let, vector<vector<string> >* slAnalysisTokens,
     vector<vector<string> >* tlAnalysisTokens,
     map<string, vector<vector<string> > > attrs, map<string, string>* vars,
     vector<string> spaces, unsigned firPat, string localeId,
     map<unsigned, unsigned> paramToPattern)
{

  printNodeAttrs (let);

  xmlNode* firstChild = xml_operations::getFirstChild (let);
  xmlNode* secondChild = xml_operations::getFirstNext (firstChild);

  string secondName = xml_operations::getName (secondChild);

  vector<string> secondResult;
  if (secondName == CLIP)
    {
      secondResult = clip (secondChild, slAnalysisTokens, tlAnalysisTokens, attrs, vars,
			   spaces, firPat, localeId, paramToPattern);
    }
  else if (secondName == CONCAT)
    {
      secondResult = concat (secondChild, slAnalysisTokens, tlAnalysisTokens, attrs, vars,
			     spaces, firPat, localeId, paramToPattern);
    }
  else if (secondName == LIT_TAG)
    {
      secondResult = litTag (secondChild);
    }
  else if (secondName == LIT)
    {
      secondResult.push_back (lit (secondChild));
    }
  else if (secondName == B)
    {
      secondResult.push_back (b (secondChild, spaces, firPat, localeId, paramToPattern));
    }
  else if (secondName == CASE_OF)
    {
      secondResult.push_back (
	  caseOf (secondChild, slAnalysisTokens, tlAnalysisTokens, localeId,
		  paramToPattern));
    }
  else if (secondName == GET_CASE_FROM)
    {
      secondResult.push_back (
	  getCaseFrom (secondChild, slAnalysisTokens, tlAnalysisTokens, attrs, vars,
		       spaces, firPat, localeId, paramToPattern));
    }
  else if (secondName == VAR)
    {
      secondResult.push_back (var (secondChild, vars));
    }

  string firstName = xml_operations::getName (firstChild);
  if (firstName == VAR)
    {
      string resultStr;
      for (unsigned i = 0; i < secondResult.size (); i++)
	resultStr += secondResult[i];

      string varName = xml_operations::getAttVal (firstChild, N);
//      cout << "varname=" << varName << " , value=" << resultStr << endl;
      (*vars)[varName] = resultStr;
//      cout << "varname=" << varName << " , value=" << (*vars)[varName] << endl;
    }
  else if (firstName == CLIP)
    {
      vector<string> firstResult = clip (firstChild, slAnalysisTokens, tlAnalysisTokens,
					 attrs, vars, spaces, firPat, localeId,
					 paramToPattern);
      if (firstResult.empty ())
	return;

      unsigned pos = xml_operations::xml_operations::getAttValUnsg (firstChild, POS);
      if (paramToPattern.size ())
	pos = paramToPattern[pos];
      pos--;

      vector<vector<string> >* analysisTokens = slAnalysisTokens;
      string side = xml_operations::getAttVal (firstChild, SIDE);
      if (side == TL)
	analysisTokens = tlAnalysisTokens;

      vector<string>* analysisToken = &((*analysisTokens)[pos]);
      // exchange the first part with the second part
      for (unsigned i = 0; i < analysisToken->size (); i++)
	{
	  if ((*analysisToken)[i] == firstResult[0])
	    {
	      analysisToken->erase (analysisToken->begin () + i,
				    analysisToken->begin () + i + firstResult.size ());
	      analysisToken->insert (analysisToken->begin () + i, secondResult.begin (),
				     secondResult.end ());
	      break;
	    }
	}
    }
}

// put the token and its tags in one vector and put tags between "<" , ">"
// the analysis will be done on this vector , "<>" to differ between tags and non-tags
// and the token for the lemma
vector<string>
formatTokenTags (string token, vector<string> tags)
{

  vector<string> analysisToken;
  analysisToken.push_back (token);

  for (unsigned i = 0; i < tags.size (); i++)
    {
      analysisToken.push_back ("<" + tags[i] + ">");
    }

  return analysisToken;
}

vector<string>
clip (xmlNode* clip, vector<vector<string> >* slAnalysisTokens,
      vector<vector<string> >* tlAnalysisTokens,
      map<string, vector<vector<string> > > attrs, map<string, string>* vars,
      vector<string> spaces, unsigned firPat, string localeId,
      map<unsigned, unsigned> paramToPattern, vector<vector<string> > tags)
{

  printNodeAttrs (clip);

  vector<string> result;

  unsigned pos = xml_operations::xml_operations::getAttValUnsg (clip, POS);
  if (paramToPattern.size ())
    pos = paramToPattern[pos];
  pos--;

  string part = xml_operations::getAttVal (clip, PART);

  if (!xml_operations::getAttVal (clip, LINK_TO).empty ())
    {
      result.push_back ("<" + xml_operations::getAttVal (clip, LINK_TO) + ">");

      return result;
    }

  string side = xml_operations::getAttVal (clip, SIDE);

  vector<string> analysisToken = (*slAnalysisTokens)[pos];

  if (side == TL)
    analysisToken = (*tlAnalysisTokens)[pos];

//  cout << "analysisToken = ";
//  for (unsigned i = 0; i < analysisToken.size (); i++)
//    cout << analysisToken[i] << " ";
//  cout << endl;

  if (part == WHOLE)
    {
      result = analysisToken;
    }
  else if (part == LEM)
    {
      result.push_back (analysisToken[0]);
    }
  else if (part == LEMH || part == LEMQ)
    {
      string lem = analysisToken[0];

      size_t spaceInd = lem.find ('#');

      if (spaceInd == string::npos)
	{
	  if (part == LEMH)
	    result.push_back (lem);
	  else
	    result.push_back ("");
	}
      else
	{
	  string lemh = lem.substr (0, spaceInd);
	  string lemq = lem.substr (spaceInd);

	  if (part == LEMH)
	    result.push_back (lemh);
	  else
	    result.push_back (lemq);
	}
    }
  else if (part == TAGS)
    {
      result.insert (result.end (), analysisToken.begin () + 1, analysisToken.end ());
    }
  // part == "attr"
  else
    {
      result = findAttrPart (analysisToken, attrs[part]);
    }

  return result;
}

vector<string>
concat (xmlNode* concat, vector<vector<string> >* slAnalysisTokens,
	vector<vector<string> >* tlAnalysisTokens,
	map<string, vector<vector<string> > > attrs, map<string, string>* vars,
	vector<string> spaces, unsigned firPat, string localeId,
	map<unsigned, unsigned> paramToPattern, vector<vector<string> > tags)
{

  printNodeAttrs (concat);

  vector<string> concatResult;

  for (xmlNode* node = xml_operations::getFirstChild (concat); node; node =
      xml_operations::getFirstNext (node))
    {
      vector<string> result;

      string nodeName = xml_operations::getName (node);
      if (nodeName == CLIP)
	{
	  result = clip (node, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces,
			 firPat, localeId, paramToPattern, tags);
	}
      else if (nodeName == LIT_TAG)
	{
	  result = litTag (node);
	}
      else if (nodeName == LIT)
	{
	  result.push_back (lit (node));
	}
      else if (nodeName == GET_CASE_FROM)
	{
	  result.push_back (
	      getCaseFrom (node, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces,
			   firPat, localeId, paramToPattern));
	}
      else if (nodeName == CASE_OF)
	{
	  result.push_back (
	      caseOf (node, slAnalysisTokens, tlAnalysisTokens, localeId,
		      paramToPattern));
	}
      else if (nodeName == B)
	{
	  result.push_back (b (node, spaces, firPat, localeId, paramToPattern));
	}
      else if (nodeName == VAR)
	{
	  result.push_back (var (node, vars));
	}

      concatResult.insert (concatResult.end (), result.begin (), result.end ());
    }

  return concatResult;
}

void
append (xmlNode* append, vector<vector<string> >* slAnalysisTokens,
	vector<vector<string> >* tlAnalysisTokens,
	map<string, vector<vector<string> > > attrs, map<string, string>* vars,
	vector<string> spaces, unsigned firPat, string localeId,
	map<unsigned, unsigned> paramToPattern)
{

  printNodeAttrs (append);

  string varName = xml_operations::getAttVal (append, NAME);

  vector<string> result;

  for (xmlNode* child = xml_operations::getFirstChild (append); child; child =
      xml_operations::getFirstNext (child))
    {
      string childName = xml_operations::getName (child);
      if (childName == CLIP)
	{
	  vector<string> clipResult = clip (child, slAnalysisTokens, tlAnalysisTokens,
					    attrs, vars, spaces, firPat, localeId,
					    paramToPattern);
	  result.insert (result.end (), clipResult.begin (), clipResult.end ());
	}
      else if (childName == LIT_TAG)
	{
	  vector<string> litTagResult = litTag (child);
	  result.insert (result.end (), litTagResult.begin (), litTagResult.end ());
	}
      else if (childName == LIT)
	{
	  string litResult = lit (child);
	  result.push_back (litResult);
	}
      else if (childName == VAR)
	{
	  string varResult = var (child, vars);
	  result.push_back (varResult);
	}
      else if (childName == CONCAT)
	{
	  vector<string> concatResult = concat (child, slAnalysisTokens, tlAnalysisTokens,
						attrs, vars, spaces, firPat, localeId,
						paramToPattern);
	  result.insert (result.end (), concatResult.begin (), concatResult.end ());
	}
      else if (childName == B)
	{
	  string bResult = b (child, spaces, firPat, localeId, paramToPattern);
	  result.push_back (bResult);
	}
      else if (childName == GET_CASE_FROM)
	{
	  string getCaseFromResult = getCaseFrom (child, slAnalysisTokens,
						  tlAnalysisTokens, attrs, vars, spaces,
						  firPat, localeId, paramToPattern);
	  result.push_back (getCaseFromResult);
	}
      else if (childName == CASE_OF)
	{
	  string caseOfResult = caseOf (child, slAnalysisTokens, tlAnalysisTokens,
					localeId, paramToPattern);
	  result.push_back (caseOfResult);
	}

    }

  string newVarValue = (*vars)[varName];
  for (unsigned i = 0; i < result.size (); i++)
    {
      newVarValue += result[i];
    }
  (*vars)[varName] = newVarValue;
}

string
b (xmlNode* b, vector<string> spaces, unsigned firPat, string localeId,
   map<unsigned, unsigned> paramToPattern)
{

  printNodeAttrs (b);

  string blank;
  if (!xml_operations::getAttVal (b, POS).empty ())
    {
      unsigned pos = xml_operations::xml_operations::getAttValUnsg (b, POS);
      if (paramToPattern.size ())
	pos = paramToPattern[pos];
      pos--;

      unsigned spacePos = firPat + (pos);
      blank = spaces[spacePos];
    }
  else
    {
      blank = " ";
    }
  return blank;
}

string
caseOf (xmlNode* caseOf, vector<vector<string> >* slAnalysisTokens,
	vector<vector<string> >* tlAnalysisTokens, string localeId,
	map<unsigned, unsigned> paramToPattern)
{

  printNodeAttrs (caseOf);

  string Case;

  unsigned pos = xml_operations::xml_operations::getAttValUnsg (caseOf, POS);
  if (paramToPattern.size ())
    pos = paramToPattern[pos];
  pos--;

  string part = xml_operations::getAttVal (caseOf, PART);

  if (part == LEM)
    {
      string side = xml_operations::getAttVal (caseOf, SIDE);

      string token;
      if (side == SL)
	token = (*slAnalysisTokens)[pos][0];
      else
	token = (*tlAnalysisTokens)[pos][0];

      if (token == CLExec::toLowerCase (token, localeId))
	Case = aa;
      else if (token == CLExec::toUpperCase (token, localeId))
	Case = AA;
      else
	Case = Aa;
    }

  return Case;
}

string
getCaseFrom (xmlNode* getCaseFrom, vector<vector<string> >* slAnalysisTokens,
	     vector<vector<string> >* tlAnalysisTokens,
	     map<string, vector<vector<string> > > attrs, map<string, string>* vars,
	     vector<string> spaces, unsigned firPat, string localeId,
	     map<unsigned, unsigned> paramToPattern)
{

  printNodeAttrs (getCaseFrom);

  string result;

  unsigned pos = xml_operations::xml_operations::getAttValUnsg (getCaseFrom, POS);
  if (paramToPattern.size ())
    pos = paramToPattern[pos];
  pos--;

  xmlNode* child = xml_operations::getFirstChild (getCaseFrom);
  string childName = xml_operations::getName (child);

  if (childName == LIT)
    {
      result = lit (child);
    }
  else if (childName == VAR)
    {
      result = var (child, vars);
    }
  else if (childName == CLIP)
    {
      vector<string> clipResult = clip (child, slAnalysisTokens, tlAnalysisTokens, attrs,
					vars, spaces, firPat, localeId, paramToPattern);

      for (unsigned i = 0; i < clipResult.size (); i++)
	{
	  result += clipResult[i];
	}
    }

  string slToken = (*slAnalysisTokens)[pos][0];

  if (slToken == CLExec::toLowerCase (slToken, localeId))
    result = CLExec::toLowerCase (result, localeId);
  else if (slToken == CLExec::toUpperCase (slToken, localeId))
    result = CLExec::toUpperCase (result, localeId);
  else
    result = CLExec::FirLetUpperCase (result, localeId);

  return result;
}

void
modifyCase (xmlNode* modifyCase, vector<vector<string> >* slAnalysisTokens,
	    vector<vector<string> >* tlAnalysisTokens,
	    map<string, vector<vector<string> > > attrs, map<string, string>* vars,
	    vector<string> spaces, unsigned firPat, string localeId,
	    map<unsigned, unsigned> paramToPattern)
{

  printNodeAttrs (modifyCase);

  xmlNode* firstChild = xml_operations::getFirstChild (modifyCase);
  xmlNode* secondChild = xml_operations::getFirstNext (firstChild);

  string childName = xml_operations::getName (secondChild);

  string Case;
  if (childName == LIT)
    {
      Case = lit (secondChild);
    }
  else if (childName == VAR)
    {
      Case = var (secondChild, vars);
    }

  childName = xml_operations::getName (firstChild);
  if (childName == VAR)
    {
      string varName = xml_operations::getAttVal (firstChild, N);

      if (Case == aa)
	(*vars)[varName] = CLExec::toLowerCase ((*vars)[varName], localeId);
      else if (Case == AA)
	(*vars)[varName] = CLExec::toUpperCase ((*vars)[varName], localeId);
      else if (Case == Aa)
	(*vars)[varName] = CLExec::FirLetUpperCase ((*vars)[varName], localeId);

    }
  else if (childName == CLIP)
    {
      unsigned pos = xml_operations::xml_operations::getAttValUnsg (firstChild, POS);
      if (paramToPattern.size ())
	pos = paramToPattern[pos];
      pos--;

      string side = xml_operations::getAttVal (firstChild, SIDE);

      vector<vector<string> >* analysisTokens;
      if (side == SL)
	analysisTokens = slAnalysisTokens;
      else
	analysisTokens = tlAnalysisTokens;

      string part = xml_operations::getAttVal (firstChild, PART);

      if (part == LEM)
	{
	  if (Case == aa)
	    (*analysisTokens)[pos][0] = CLExec::toLowerCase ((*analysisTokens)[pos][0],
							     localeId);
	  else if (Case == AA)
	    (*analysisTokens)[pos][0] = CLExec::toUpperCase ((*analysisTokens)[pos][0],
							     localeId);
	  else if (Case == Aa)
	    (*analysisTokens)[pos][0] = CLExec::FirLetUpperCase (
		(*analysisTokens)[pos][0], localeId);
	}
      else if (part == LEMH || part == LEMQ)
	{
	  string lem = (*analysisTokens)[pos][0];

	  size_t spaceInd = lem.find ('#');
	  if (spaceInd == string::npos)
	    {
	      if (Case == aa)
		lem = CLExec::toLowerCase (lem, localeId);
	      else if (Case == AA)
		lem = CLExec::toUpperCase (lem, localeId);
	      else if (Case == Aa)
		lem = CLExec::FirLetUpperCase (lem, localeId);
	    }
	  else
	    {
	      string lemh = lem.substr (0, spaceInd);
	      string lemq = lem.substr (spaceInd);

	      if (part == LEMH)
		{
		  if (Case == aa)
		    lemh = CLExec::toLowerCase (lemh, localeId);
		  else if (Case == AA)
		    lemh = CLExec::toUpperCase (lemh, localeId);
		  else if (Case == Aa)
		    lemh = CLExec::FirLetUpperCase (lemh, localeId);
		}
	      else
		{
		  if (Case == aa)
		    lemq = CLExec::toLowerCase (lemq, localeId);
		  else if (Case == AA)
		    lemq = CLExec::toUpperCase (lemq, localeId);
		  else if (Case == Aa)
		    lemq = CLExec::FirLetUpperCase (lemq, localeId);
		}

	      lem = lemh + lemq;
	    }
	}

    }

}
