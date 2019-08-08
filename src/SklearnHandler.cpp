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

int main(int argc, char **argv) {
	string localeId, lextorFilePath, transferFilePath, chunkerFilePath,
			predictDataFilePath, predictResFilePath;

	int opt;
	bool g = false, p = false;
	while ((opt = getopt(argc, argv, ":g:p:")) != -1) {
		switch (opt) {
		case 'g':
			predictDataFilePath = optarg;
			g = true;
			break;
		case 'p':
			predictResFilePath = optarg;
			p = true;
			break;
		case ':':
			printf("option %c needs a value\n", optopt);
			return -1;
		case '?':
			printf("unknown option: %c\n", optopt);
			return -1;
		}
	}

	if (p && !g && argc - optind == 4) {
		localeId = argv[argc - 4];
		transferFilePath = argv[argc - 3];
		lextorFilePath = argv[argc - 2];
		chunkerFilePath = argv[argc - 1];

	} else if (g && !p && argc - optind == 3) {
		localeId = argv[argc - 3];
		transferFilePath = argv[argc - 2];
		lextorFilePath = argv[argc - 1];
	} else {
		localeId = "es_ES";
		transferFilePath = "apertium-eng-spa.spa-eng.t1x";
		lextorFilePath = "lextor.txt";
		predictDataFilePath = "predict-data";

//      localeId = "kk_KZ";
//      transferFilePath = "apertium-kaz-tur.kaz-tur.t1x";
//      sentenceFilePath = "src.txt";
//      lextorFilePath = "lextor.txt";
//      interInFilePath = "beam-inter.txt";
//      modelsDest = "./UntitledFolder/models";
//      k = "8";

		cout << "Error in parameters !" << endl;
		cout
				<< "Parameters are : localeId transferFilePath lextorFilePath [(-g predictDataFilePath) || (-p predictResFilePath chunkerFilePath)]"
				<< endl;
		cout
				<< "localeId : ICU locale ID for the source language. For Kazakh => kk_KZ"
				<< endl;
		cout
				<< "transferFilePath : Apertium transfer file of the language pair used."
				<< endl;
		cout
				<< "lextorFilePath : Apertium lextor file for the source language sentences."
				<< endl;
		cout
				<< "-g : Data generation mode (expects file destination to put datasets in)."
				<< endl;
		cout
				<< "predictDataFilePath : destination to put in the generated sklearn predict data."
				<< endl;
		cout << "-p : Prediction mode (expects prediction results file)."
				<< endl;
		cout << "predictResFilePath : File that holds prediction results."
				<< endl;
		return -1;
	}

	ifstream lextorFile(lextorFilePath.c_str());
	ofstream predictDataFile(predictDataFilePath.c_str());
	ifstream predictResFile(predictResFilePath.c_str());
	ofstream chunkerFile(chunkerFilePath.c_str());
	if (lextorFile.is_open()
			&& (predictDataFilePath.empty() || predictDataFile.is_open())
			&& (predictResFilePath.empty() || predictResFile.is_open())
			&& (chunkerFilePath.empty() || chunkerFile.is_open())) {
		// load transfer file in an xml document object
		xml_document transferDoc;
		xml_parse_result result = transferDoc.load_file(
				transferFilePath.c_str());
		if (string(result.description()) != "No error") {
			cout << "ERROR : " << result.description() << endl;
			return -1;
		}

		// xml node of the parent node (transfer) in the transfer file
		xml_node transfer = transferDoc.child("transfer");

		map<string, vector<vector<string> > > attrs = RuleParser::getAttrs(
				transfer);
		map<string, string> vars = RuleParser::getVars(transfer);
		map<string, vector<string> > lists = RuleParser::getLists(transfer);

		unsigned i = 0;
		string tokenizedSentence;
		while (getline(lextorFile, tokenizedSentence)) {
			cout << i++ << endl;

			// spaces after each token
			vector<string> spaces;

			// tokens in the sentence order
			vector<string> slTokens, tlTokens;
			// tags of tokens in order
			vector<vector<string> > slTags, tlTags;
			RuleParser::sentenceTokenizer(&slTokens, &tlTokens, &slTags,
					&tlTags, &spaces, tokenizedSentence);
			// map of tokens ids and their matched categories
			map<unsigned, vector<string> > catsApplied;

			RuleParser::matchCats(&catsApplied, slTokens, slTags, transfer);
			// map of matched rules and a pair of first token id and patterns number
			map<xml_node, vector<pair<unsigned, unsigned> > > rulesApplied;
			RuleParser::matchRules(&rulesApplied, slTokens, catsApplied,
					transfer);
			// rule and (target) token map to specific output
			// if rule has many patterns we will choose the first token only
			map<unsigned, map<unsigned, string> > ruleOutputs;
			// map (target) token to all matched rules ids and the number of pattern items of each rule
			map<unsigned, vector<pair<unsigned, unsigned> > > tokenRules;

			RuleExecution::ruleOuts(&ruleOutputs, &tokenRules, slTokens, slTags,
					tlTokens, tlTags, rulesApplied, attrs, lists, &vars, spaces,
					localeId);
			// number of generated combinations
			unsigned compNum;
			// nodes for every token and rule
			map<unsigned, vector<RuleExecution::Node*> > nodesPool;
			// ambiguous informations
			vector<RuleExecution::AmbigInfo*> ambigInfo;

			nodesPool = RuleExecution::getNodesPool(tokenRules);
			RuleExecution::getAmbigInfo(tokenRules, nodesPool, &ambigInfo,
					&compNum);
			vector<RuleExecution::AmbigInfo*> newAmbigInfo;
			for (unsigned j = 0; j < ambigInfo.size(); j++)
				if (ambigInfo[j]->combinations.size() > 1)
					newAmbigInfo.push_back(ambigInfo[j]);

			// generation mode
			if (g)
				for (unsigned j = 0; j < newAmbigInfo.size(); j++) {
					RuleExecution::AmbigInfo* ambig = newAmbigInfo[j];

					string rulesNums;
					for (unsigned x = 0; x < ambig->combinations.size(); x++) {
						// avoid dummy node
						for (unsigned y = 1; y < ambig->combinations[x].size();
								y++) {
							stringstream ss;
							ss << ambig->combinations[x][y]->ruleId;
							rulesNums += ss.str();

							if (y + 1 < ambig->combinations[x].size())
								rulesNums += "_";
						}
						rulesNums += "+";
					}
					// write the record
					predictDataFile << rulesNums << " ";
					for (unsigned x = ambig->firTokId;
							x < ambig->firTokId + ambig->maxPat; x++) {

						for (unsigned t = 0; t < slTokens[x].size(); t++) {
							// remove '#' and put '_'
							if (slTokens[x][t] == '#')
								if (t + 1 < slTokens[x].length()
										&& slTokens[x][t] == ' ')
									slTokens[x].replace(t--, 1, "");
								else
									slTokens[x].replace(t, 1, "_");

							// remove ' ' and put '_'
							else if (slTokens[x][t] == ' ')
								slTokens[x].replace(t, 1, "_");
						}

						predictDataFile
								<< CLExec::toLowerCase(slTokens[x], localeId)
								<< " ";
					}
					predictDataFile << endl;
				}
			// predict mode
			else if (p) {
				vector<RuleExecution::Node*> finalNodes;
				unsigned j = 0;
				for (unsigned x = 0; x < slTokens.size();) {
					if (j < newAmbigInfo.size()
							&& x == newAmbigInfo[j]->firTokId) {
						string line;
						getline(predictResFile, line);
						int rule;
						stringstream buffer(line);
						buffer >> rule;

						// skip dummy node
						finalNodes.insert(finalNodes.end(),
								newAmbigInfo[j]->combinations[rule].begin() + 1,
								newAmbigInfo[j]->combinations[rule].end());

						x += newAmbigInfo[j]->maxPat;
						j++;
					} else {
						finalNodes.push_back(nodesPool[x][0]);
						x++;
					}
				}
				string out;
				for (unsigned x = 0; x < finalNodes.size(); x++) {
					out +=
							ruleOutputs[finalNodes[x]->ruleId][finalNodes[x]->tokenId]
									+ spaces[finalNodes[x]->tokenId
											+ finalNodes[x]->patNum - 1];

				}
				chunkerFile << out << endl;
			}

			// delete AmbigInfo pointers
			for (unsigned j = 0; j < ambigInfo.size(); j++) {
				// delete the dummy node pointers
				set<RuleExecution::Node*> dummies;
				for (unsigned k = 0; k < ambigInfo[j]->combinations.size(); k++)
					dummies.insert(ambigInfo[j]->combinations[k][0]);
				for (set<RuleExecution::Node*>::iterator it = dummies.begin();
						it != dummies.end(); it++)
					delete (*it);

				delete ambigInfo[j];
			}
			// delete Node pointers
			for (map<unsigned, vector<RuleExecution::Node*> >::iterator it =
					nodesPool.begin(); it != nodesPool.end(); it++) {
				for (unsigned j = 0; j < it->second.size(); j++) {
					delete it->second[j];
				}
			}
		}
		lextorFile.close();
		predictDataFile.close();
		predictResFile.close();
		chunkerFile.close();
	} else {
		cout << "ERROR in opening files!" << endl;
	}
	return 0;
}
