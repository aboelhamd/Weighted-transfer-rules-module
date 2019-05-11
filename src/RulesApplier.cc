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
#include <libxml2/libxml/parser.h>
#include <libxml2/libxml/tree.h>

#include "ambiguous_transfer.h"
#include "transfer_literals.h"
#include "case_handler.h"

using namespace std;
using namespace elem;

int main(int argc, char **argv) {
	string localeId, transferFilePath, lextorFilePath, interInFilePath;

	if (argc == 5) {
		localeId = argv[1];
		transferFilePath = argv[2];
		lextorFilePath = argv[3];
		interInFilePath = argv[4];
	} else {
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
		transferFilePath = "apertium-eng-spa.spa-eng.t1x";
		lextorFilePath = "lextor.txt";
		interInFilePath = "interIn.txt";

		cout << "Error in parameters !" << endl;
		cout
				<< "Parameters are : localeId transferFilePath lextorFilePath interInFilePath"
				<< endl;
		cout
				<< "localeId : ICU locale ID for the source language. For Kazakh => kk-KZ"
				<< endl;
		cout
				<< "transferFilePath : Apertium transfer file of the language pair used."
				<< endl;
		cout
				<< "lextorFilePath : Apertium lextor file for the source language sentences."
				<< endl;
		cout
				<< "interInFilePath : Output file name of this program which is the input for apertium interchunk."
				<< endl;
//		return -1;
	}

	ifstream lextorFile(lextorFilePath.c_str());
	if (lextorFile.is_open()) {

		xmlDoc* doc = xmlReadFile(transferFilePath.c_str(), NULL, 0);

		if (doc == NULL) {
			cerr << "Error: Could not parse file \'" << transferFilePath
					<< "\'." << endl;
			exit(EXIT_FAILURE);
		}

		xmlNode* transfer = xmlDocGetRootElement(doc);

		vector<string> tokenizedSentences;

		string tokenizedSentence;
		while (getline(lextorFile, tokenizedSentence)) {
			tokenizedSentences.push_back(tokenizedSentence);
		}
		lextorFile.close();

		map<string, vector<vector<string> > > attrs =
				AmbiguousChunker::getAttrs(transfer);
		map<string, string> vars = AmbiguousChunker::getVars(transfer);
		map<string, vector<string> > lists = AmbiguousChunker::getLists(
				transfer);

		ofstream interInFile(interInFilePath.c_str());
		if (interInFile.is_open())
			for (unsigned i = 0; i < tokenizedSentences.size(); i++) {
				cout << i << endl;

				string tokenizedSentence;
				tokenizedSentence = tokenizedSentences[i];

				// spaces after each token
				vector<string> spaces;

				// tokens in the sentence order
				vector<string> slTokens, tlTokens;

				// tags of tokens in order
				vector<vector<string> > slTags, tlTags;

				AmbiguousChunker::lexFormsTokenizer(&slTokens, &tlTokens,
						&slTags, &tlTags, &spaces, tokenizedSentence);

				// map of tokens ids and their matched categories
				map<unsigned, vector<string> > catsApplied;

				AmbiguousChunker::matchCats(&catsApplied, slTokens, slTags,
						transfer);

				// map of matched rules and a pair of first token id and patterns number
				map<xmlNode*, vector<pair<unsigned, unsigned> > > rulesApplied;

				AmbiguousChunker::matchRules(&rulesApplied, slTokens,
						catsApplied, transfer);

				// rule and (target) token map to specific output
				// if rule has many patterns we will choose the first token only
				map<unsigned, map<unsigned, string> > ruleOutputs;

				// map (target) token to all matched rules ids and the number of pattern items of each rule
				map<unsigned, vector<pair<unsigned, unsigned> > > tokenRules;

				AmbiguousChunker::ruleOuts(&ruleOutputs, &tokenRules, slTokens,
						slTags, tlTokens, tlTags, rulesApplied, attrs, lists,
						&vars, spaces);
				// final outs
				vector<string> outs;
				// number of possible combinations
				unsigned compNum;
				// nodes for every token and rule
				map<unsigned, vector<AmbiguousChunker::Node> > nodesPool;
				// ambiguous informations
				vector<AmbiguousChunker::AmbigInfo> ambigInfo;
				// rules combinations
				vector<vector<AmbiguousChunker::Node> > combNodes;

				nodesPool = AmbiguousChunker::getNodesPool(tokenRules);

//				for (map<unsigned, map<unsigned, string> >::iterator it =
//						ruleOutputs.begin(); it != ruleOutputs.end(); it++) {
//					cout << "ruleId=" << it->first << endl;
//					map<unsigned, string> outs = it->second;
//
//					for (map<unsigned, string>::iterator it2 = outs.begin();
//							it2 != outs.end(); it2++) {
//						cout << "tokId=" << it2->first << " , out = "
//								<< it2->second << endl;
//					}
//					cout << endl;
//				}
//				cout << endl;
//
//				for (unsigned j = 0; j < tlTokens.size(); j++) {
//					vector<Node> nodes = nodesPool[j];
//					cout << "tokId = " << j << " : " << tlTokens[j] << endl;
//					for (unsigned k = 0; k < nodes.size(); k++) {
//						cout << "ruleId = " << nodes[k].ruleId << "; patNum = "
//								<< nodes[k].patNum << endl;
//					}
//					cout << endl;
//				}

				AmbiguousChunker::getAmbigInfo(tokenRules, nodesPool,
						&ambigInfo, &compNum);
				AmbiguousChunker::getOuts(&outs, &combNodes, ambigInfo,
						nodesPool, ruleOutputs, spaces);

//				for (unsigned j = 0; j < combNodes.size(); j++) {
//					vector<Node> nodes = combNodes[j];
//					for (unsigned k = 0; k < nodes.size(); k++) {
//						cout << "tok=" << nodes[k].tokenId << "; rul="
//								<< nodes[k].ruleId << "; pat="
//								<< nodes[k].patNum << " - ";
//					}
//					cout << endl;
//				}

				// write the outs
				for (unsigned j = 0; j < outs.size(); j++)
					interInFile << outs[j] << endl;
			}
		else
			cout << "ERROR in opening files!" << endl;
		interInFile.close();

		cout << "RulesApplier finished!";
	} else {
		cout << "ERROR in opening files!" << endl;
	}

	return 0;
}
