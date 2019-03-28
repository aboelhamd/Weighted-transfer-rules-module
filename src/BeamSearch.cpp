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

void BeamSearch::transfer(string transferFilePath, string modelsFileDest,
		string k, FILE* lextorFile, FILE* outFile) {

	xmlDoc* doc = xmlReadFile(transferFilePath.c_str(), NULL, 0);

	if (doc == NULL) {
		cerr << "Error: Could not parse file \'" << transferFilePath << "\'."
				<< endl;
		exit(EXIT_FAILURE);
	}

	xmlNode* transfer = xmlDocGetRootElement(doc);

	map<string, vector<vector<string> > > attrs = RuleParser::getAttrs(
			transfer);
	map<string, string> vars = RuleParser::getVars(transfer);
	map<string, vector<string> > lists = RuleParser::getLists(transfer);

	string localeId;
	map<string, map<string, vector<float> > > classesWeights =
			CLExec::loadYasmetModels(modelsFileDest, &localeId);

	int beam;
	stringstream buffer(k);
	buffer >> beam;

	char buff[10240];
	string tokenizedSentence;
	while (fgets(buff, 10240, lextorFile)) {
		tokenizedSentence = buff;

		// spaces after each token
		vector<string> spaces;

		// tokens in the sentence order
		vector<string> slTokens, tlTokens;

		// tags of tokens in order
		vector<vector<string> > slTags, tlTags;

		RuleParser::sentenceTokenizer(&slTokens, &tlTokens, &slTags, &tlTags,
				&spaces, tokenizedSentence);

		// map of tokens ids and their matched categories
		map<unsigned, vector<string> > catsApplied;

		RuleParser::matchCats(&catsApplied, slTokens, slTags, transfer);

		// map of matched rules and a pair of first token id and patterns number
		map<xmlNode*, vector<pair<unsigned, unsigned> > > rulesApplied;

		RuleParser::matchRules(&rulesApplied, slTokens, catsApplied, transfer);

		// rule and (target) token map to specific output
		// if rule has many patterns we will choose the first token only
		map<unsigned, map<unsigned, string> > ruleOutputs;

		// map (target) token to all matched rules ids and the number of pattern items of each rule
		map<unsigned, vector<pair<unsigned, unsigned> > > tokenRules;

		ambiguous_transfer::ruleOuts(&ruleOutputs, &tokenRules, slTokens,
				slTags, tlTokens, tlTags, rulesApplied, attrs, lists, &vars,
				spaces, localeId);

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

		nodesPool = ambiguous_transfer::getNodesPool(tokenRules);

		ambiguous_transfer::getAmbigInfo(tokenRules, nodesPool, &ambigInfo,
				&compNum);

		vector<ambiguous_transfer::AmbigInfo> newAmbigInfo;
		for (unsigned j = 0; j < ambigInfo.size(); j++)
			if (ambigInfo[j].combinations.size() > 1)
				newAmbigInfo.push_back(ambigInfo[j]);

		CLExec::beamSearch(&beamTree, beam, slTokens, newAmbigInfo,
				classesWeights, localeId);

		ambiguous_transfer::getOuts(&outs, &combNodes, beamTree, nodesPool,
				ruleOutputs, spaces);

		// write the outs
		for (unsigned j = 0; j < outs.size(); j++) {
			fputs(outs[j].c_str(), outFile);
		}
	}

}

int main(int argc, char **argv) {
	cout << "hereeeeeeeeeeeeeeeee" << endl;
}
