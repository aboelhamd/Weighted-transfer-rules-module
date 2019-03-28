/*
 * ambiguous_transfer.h
 *
 *  Created on: May 5, 2018
 *      Author: aboelhamd
 */

#ifndef SRC_AMBIGUOUS_TRANSFER_H_
#define SRC_AMBIGUOUS_TRANSFER_H_

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <algorithm>
#include "TranElemLiterals.h"
#include "xml_operations.h"

using namespace std;

class ambiguous_transfer {
public:
	class Node {
	public:
		unsigned tokenId;
		unsigned ruleId;
		unsigned patNum;
		vector<Node> neighbors;
		Node(unsigned tokenId, unsigned ruleId, unsigned patNum) {
			this->tokenId = tokenId;
			this->ruleId = ruleId;
			this->patNum = patNum;
		}
		Node() {
			this->tokenId = 0;
			this->ruleId = 0;
			this->patNum = 0;
		}
	};

	class AmbigInfo {
	public:
		unsigned firTokId;
		unsigned maxPat;
		vector<vector<Node> > combinations;
		AmbigInfo(unsigned firTokId, unsigned maxPat) {
			this->firTokId = firTokId;
			this->maxPat = maxPat;
		}
		AmbigInfo() {
			this->firTokId = 0;
			this->maxPat = 0;
		}
	};

	static void
	normaliseWeights(vector<float>* weights, vector<AmbigInfo> ambigInfo);

	static map<unsigned, vector<Node> >
	getNodesPool(map<unsigned, vector<pair<unsigned, unsigned> > > tokenRules);

	static void
	getAmbigInfo(map<unsigned, vector<pair<unsigned, unsigned> > > tokenRules,
			map<unsigned, vector<Node> > nodesPool,
			vector<AmbigInfo>* ambigInfo, unsigned* combNum);

	static void
	ruleOuts(map<unsigned, map<unsigned, string> >* ruleOuts,
			map<unsigned, vector<pair<unsigned, unsigned> > >* tokenRules,
			vector<string> slTokens, vector<vector<string> > slTags,
			vector<string> tlTokens, vector<vector<string> > tlTags,
			map<xmlNode*, vector<pair<unsigned, unsigned> > > rulesApplied,
			map<string, vector<vector<string> > > attrs,
			map<string, vector<string> > lists, map<string, string>* vars,
			vector<string> spaces, string localeId);

	static void
	getOuts(vector<string>* finalOuts,
			vector<vector<ambiguous_transfer::Node> >* finalCombNodes,
			vector<pair<vector<ambiguous_transfer::Node>, float> > beamTree,
			map<unsigned, vector<ambiguous_transfer::Node> > nodesPool,
			map<unsigned, map<unsigned, string> > ruleOutputs,
			vector<string> spaces);

	static void
	getOuts(vector<string>* finalOuts,
			vector<vector<ambiguous_transfer::Node> >* combNodes,
			vector<ambiguous_transfer::AmbigInfo> ambigInfo,
			map<unsigned, vector<ambiguous_transfer::Node> > nodesPool,
			map<unsigned, map<unsigned, string> > ruleOutputs,
			vector<string> spaces);
};

string
noRuleOut(vector<string> analysis);

void
putOut(vector<string>* outputs, string output, unsigned tokenIndex,
		vector<string> spaces);

vector<string>
putOuts(vector<string> outputs, vector<string> nestedOutputs);

void
putCombination(vector<vector<ambiguous_transfer::Node> >* combinations,
		vector<ambiguous_transfer::Node> combination);

vector<vector<ambiguous_transfer::Node> >
putCombinations(vector<vector<ambiguous_transfer::Node> > combinations,
		vector<vector<ambiguous_transfer::Node> > nestedcombinations);

ambiguous_transfer::Node
ambiguousGraph(map<unsigned, vector<pair<unsigned, unsigned> > > tokenRules,
		map<unsigned, vector<ambiguous_transfer::Node> > nodesPool,
		unsigned firTok, unsigned maxPat);

ambiguous_transfer::Node
ambiguousGraph(map<unsigned, vector<pair<unsigned, unsigned> > > tokenRules,
		map<unsigned, vector<ambiguous_transfer::Node> > nodesPool);

bool
outputs(vector<string>* outs,
		vector<vector<pair<unsigned, unsigned> > >* rulesIds,
		vector<vector<vector<unsigned> > >* outsRules,
		vector<
				pair<pair<unsigned, unsigned>,
						pair<unsigned, vector<vector<unsigned> > > > > *ambigInfo,
		vector<string> tlTokens, vector<vector<string> > tags,
		map<unsigned, map<unsigned, string> > ruleOuts,
		map<unsigned, vector<pair<unsigned, unsigned> > > tokenRules,
		vector<string> spaces);

vector<string>
ruleExe(xmlNode* rule, vector<vector<string> >* slAnalysisTokens,
		vector<vector<string> >* tlAnalysisTokens,
		map<string, vector<vector<string> > > attrs,
		map<string, vector<string> > lists, map<string, string>* vars,
		vector<string> spaces, unsigned firPat, string localeId);

vector<string>
choose(xmlNode* choose, vector<vector<string> >* slAnalysisTokens,
		vector<vector<string> >* tlAnalysisTokens,
		map<string, vector<vector<string> > > attrs,
		map<string, vector<string> > lists, map<string, string>* vars,
		vector<string> spaces, unsigned firPat, string localeId,
		map<unsigned, unsigned> paramToPattern);

void
let(xmlNode* let, vector<vector<string> >* slAnalysisTokens,
		vector<vector<string> >* tlAnalysisTokens,
		map<string, vector<vector<string> > > attrs, map<string, string>* vars,
		vector<string> spaces, unsigned firPat, string localeId,
		map<unsigned, unsigned> paramToPattern);

vector<string>
callMacro(xmlNode* callMacro, vector<vector<string> >* slAnalysisTokens,
		vector<vector<string> >* tlAnalysisTokens,
		map<string, vector<vector<string> > > attrs,
		map<string, vector<string> > lists, map<string, string>* vars,
		vector<string> spaces, unsigned firPat, string localeId,
		map<unsigned, unsigned> paramToPattern);

vector<string>
out(xmlNode* out, vector<vector<string> >* slAnalysisTokens,
		vector<vector<string> >* tlAnalysisTokens,
		map<string, vector<vector<string> > > attrs, map<string, string>* vars,
		vector<string> spaces, unsigned firPat, string localeId,
		map<unsigned, unsigned> paramToPattern);

vector<string>
chunk(xmlNode* chunkNode, vector<vector<string> >* slAnalysisTokens,
		vector<vector<string> >* tlAnalysisTokens,
		map<string, vector<vector<string> > > attrs, map<string, string>* vars,
		vector<string> spaces, unsigned firPat, string localeId,
		map<unsigned, unsigned> paramToPattern);

vector<string>
formatTokenTags(string token, vector<string> tags);

vector<string>
findAttrPart(vector<string> tokenTags, vector<vector<string> > attrTags);

vector<string>
clip(xmlNode* clip, vector<vector<string> >* slAnalysisTokens,
		vector<vector<string> >* tlAnalysisTokens,
		map<string, vector<vector<string> > > attrs, map<string, string>* vars,
		vector<string> spaces, unsigned firPat, string localeId,
		map<unsigned, unsigned> paramToPattern, vector<vector<string> > tags =
				vector<vector<string> >());

vector<string>
concat(xmlNode* concat, vector<vector<string> >* slAnalysisTokens,
		vector<vector<string> >* tlAnalysisTokens,
		map<string, vector<vector<string> > > attrs, map<string, string>* vars,
		vector<string> spaces, unsigned firPat, string localeId,
		map<unsigned, unsigned> paramToPattern, vector<vector<string> > tags =
				vector<vector<string> >());

bool
equal(xmlNode* equal, vector<vector<string> >* slAnalysisTokens,
		vector<vector<string> >* tlAnalysisTokens,
		map<string, vector<vector<string> > > attrs, map<string, string>* vars,
		vector<string> spaces, unsigned firPat, string localeId,
		map<unsigned, unsigned> paramToPattern);

bool
test(xmlNode* test, vector<vector<string> >* slAnalysisTokens,
		vector<vector<string> >* tlAnalysisTokens,
		map<string, vector<vector<string> > > attrs,
		map<string, vector<string> > lists, map<string, string>* vars,
		vector<string> spaces, unsigned firPat, string localeId,
		map<unsigned, unsigned> paramToPattern);

bool
And(xmlNode* And, vector<vector<string> >* slAnalysisTokens,
		vector<vector<string> >* tlAnalysisTokens,
		map<string, vector<vector<string> > > attrs,
		map<string, vector<string> > lists, map<string, string>* vars,
		vector<string> spaces, unsigned firPat, string localeId,
		map<unsigned, unsigned> paramToPattern);

bool
Or(xmlNode* Or, vector<vector<string> >* slAnalysisTokens,
		vector<vector<string> >* tlAnalysisTokens,
		map<string, vector<vector<string> > > attrs,
		map<string, vector<string> > lists, map<string, string>* vars,
		vector<string> spaces, unsigned firPat, string localeId,
		map<unsigned, unsigned> paramToPattern);

bool
in(xmlNode* in, vector<vector<string> >* slAnalysisTokens,
		vector<vector<string> >* tlAnalysisTokens,
		map<string, vector<vector<string> > > attrs,
		map<string, vector<string> > lists, map<string, string>* vars,
		vector<string> spaces, unsigned firPat, string localeId,
		map<unsigned, unsigned> paramToPattern);

bool
Not(xmlNode* Not, vector<vector<string> >* slAnalysisTokens,
		vector<vector<string> >* tlAnalysisTokens,
		map<string, vector<vector<string> > > attrs,
		map<string, vector<string> > lists, map<string, string>* vars,
		vector<string> spaces, unsigned firPat, string localeId,
		map<unsigned, unsigned> paramToPattern);

vector<string>
litTag(xmlNode* litTag);

string
lit(xmlNode* lit);

string
var(xmlNode* var, map<string, string>* vars);

void
append(xmlNode* append, vector<vector<string> >* slAnalysisTokens,
		vector<vector<string> >* tlAnalysisTokens,
		map<string, vector<vector<string> > > attrs, map<string, string>* vars,
		vector<string> spaces, unsigned firPat, string localeId,
		map<unsigned, unsigned> paramToPattern);

string
b(xmlNode* b, vector<string> spaces, unsigned firPat, string localeId,
		map<unsigned, unsigned> paramToPattern);

string
caseOf(xmlNode* caseOf, vector<vector<string> >* slAnalysisTokens,
		vector<vector<string> >* tlAnalysisTokens, string localeId,
		map<unsigned, unsigned> paramToPattern);

string
getCaseFrom(xmlNode* getCaseFrom, vector<vector<string> >* slAnalysisTokens,
		vector<vector<string> >* tlAnalysisTokens,
		map<string, vector<vector<string> > > attrs, map<string, string>* vars,
		vector<string> spaces, unsigned firPat, string localeId,
		map<unsigned, unsigned> paramToPattern);

void
modifyCase(xmlNode* modifyCase, vector<vector<string> >* slAnalysisTokens,
		vector<vector<string> >* tlAnalysisTokens,
		map<string, vector<vector<string> > > attrs, map<string, string>* vars,
		vector<string> spaces, unsigned firPat, string localeId,
		map<unsigned, unsigned> paramToPattern);

#endif /* SRC_AMBIGUOUS_TRANSFER_H_ */
