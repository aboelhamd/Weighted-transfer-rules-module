/*
 * ruleParser.h
 *
 *  Created on: Apr 25, 2018
 *      Author: aboelhamd
 */

#ifndef SRC_RULEPARSER_H_
#define SRC_RULEPARSER_H_

#include "TranElemLiterals.h"
#include "xml_operations.h"

using namespace std;

class RuleParser {
public:
	static void
	sentenceTokenizer(vector<string>* slTokens, vector<string>* tlTokens,
			vector<vector<string> >* slTags, vector<vector<string> >* tlTags,
			vector<string>* spaces, string tokenizedSentenceStr);

	static void
	matchCats(map<unsigned, vector<string> >* catsApplied,
			vector<string> slTokens, vector<vector<string> > tags,
			xmlNode* transfer);

	static void
	matchRules(map<xmlNode*, vector<pair<unsigned, unsigned> > >* rulesApplied,
			vector<string> slTokens, map<unsigned, vector<string> > catsApplied,
			xmlNode* transfer);

	static map<string, vector<vector<string> > >
	getAttrs(xmlNode* transfer);

	static map<string, string>
	getVars(xmlNode* transfer);

	static map<string, vector<string> >
	getLists(xmlNode* transfer);
};

#endif /* SRC_RULEPARSER_H_ */
