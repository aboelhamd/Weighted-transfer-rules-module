#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string.h>

#include "RuleParser.h"

using namespace std;
using namespace elem;

void RuleParser::sentenceTokenizer(vector<string>* slTokens,
		vector<string>* tlTokens, vector<vector<string> >* slTags,
		vector<vector<string> >* tlTags, vector<string>* spaces,
		string tokenizedSentenceStr) {
	vector<string> taggedTokens;
	// from string to char*
	char tokenizedSentence[tokenizedSentenceStr.size()];
	strcpy(tokenizedSentence, tokenizedSentenceStr.c_str());

	char * taggedToken;
	taggedToken = strtok(tokenizedSentence, "^");
	while (taggedToken != NULL) {
		taggedTokens.push_back(taggedToken);
		taggedToken = strtok(NULL, "^");
	}

	size_t taggedTokensSize = taggedTokens.size();
	for (unsigned i = 0; i < taggedTokensSize; i++) {
		// take spaces after token
		size_t dolSignInd = taggedTokens[i].find("$");
		spaces->push_back(taggedTokens[i].substr(dolSignInd + 1));
		taggedTokens[i] = taggedTokens[i].substr(0, dolSignInd);

		// remove multiple translations and take only the first one
		size_t firSlashInd = taggedTokens[i].find("/");

		// if no translation , remove that word
		if (firSlashInd + 1 == taggedTokens[i].size()) {
			taggedTokens.erase(taggedTokens.begin() + i);
			spaces->erase(spaces->begin() + i);
			taggedTokensSize--;
			i--;
			continue;
		}

		size_t secSlashInd = taggedTokens[i].find("/", firSlashInd + 1);
		if (secSlashInd != string::npos)
			taggedTokens[i] = taggedTokens[i].substr(0, secSlashInd);

		// split source and target tokens
		string target = taggedTokens[i].substr(firSlashInd + 1);

		taggedTokens.push_back(target);

		taggedTokens[i] = taggedTokens[i].substr(0, firSlashInd);
	}

	for (unsigned i = 0; i < taggedTokens.size(); i++) {
		char taggedToken[taggedTokens[i].size()];
		strcpy(taggedToken, taggedTokens[i].c_str());
		char* split;

		string token;
		vector<string> tokTags;

//      cout << "taggedToken : " << taggedToken << endl;

		if (taggedToken[0] != '<') {
			split = strtok(taggedToken, "<>");
			token = split;
			split = strtok(NULL, "<>");
		} else {
			split = strtok(taggedToken, "<>");
		}

//      cout << "word : " << token << endl;

		while (split != NULL) {
			string tag = split;
			tokTags.push_back(tag);

			split = strtok(NULL, "<>");
		}

		if (i < taggedTokens.size() / 2) {
			slTokens->push_back(token);
			slTags->push_back(tokTags);
		} else {
			tlTokens->push_back(token);
			tlTags->push_back(tokTags);
		}
	}
}

void RuleParser::matchCats(map<unsigned, vector<string> >* catsApplied,
		vector<string> slTokens, vector<vector<string> > tags,
		xmlNode* transfer) {
	xmlNode* section_def_cats = xml_operations::getChild(transfer, SECTION_DEF_CATS);

//  cout << "here" << endl;

	for (xmlNode* def_cat = xml_operations::getFirstChild(section_def_cats); def_cat; def_cat =
			xml_operations::getFirstNext(def_cat)) {

		for (xmlNode* cat_item = xml_operations::getFirstChild(def_cat); cat_item; cat_item =
				xml_operations::getFirstNext(cat_item)) {

			// separate tags from (t1.t2) format, for easy access
			string tagsString = xml_operations::getAttVal(cat_item, TAGS);

			char tagDotted[tagsString.size()];
			strcpy(tagDotted, tagsString.c_str());
			char* split;
			split = strtok(tagDotted, ".");

			vector<string> itemTags;

			while (split != NULL) {
				string tag = split;
				itemTags.push_back(tag);

				split = strtok(NULL, ".");
			}

			for (unsigned x = 0; x < slTokens.size(); x++) {
				// if cat-item have lemma
				if (!xml_operations::getAttVal(cat_item, LEMMA).empty()) {
					if (xml_operations::getAttVal(cat_item, LEMMA) != slTokens[x]) {
						continue;
					}
				}

				vector<string> tokTags = tags[x];

				unsigned i = 0, j = 0;
				for (; i < tokTags.size() && j < itemTags.size(); i++) {
					if (itemTags[j] == "*") {
						if (j + 1 < itemTags.size() && i + 1 < tokTags.size()
								&& itemTags[j + 1] == tokTags[i + 1]) {
							j += 2;
							i++;
						}
					} else if (itemTags[j] == tokTags[i]) {
						j++;
					} else {
						break;
					}
				}

				if (i == tokTags.size()
						&& (j == itemTags.size()
								|| (j + 1 == itemTags.size()
										&& itemTags[j] == "*"
										&& itemTags[j - 1] != tokTags[i - 1]))) {
//	    	  cout << N <<endl;
//					cout << def_cat.attribute(N).value() << endl;
//					(*catsApplied)[x];
//					(*catsApplied)[x].push_back("");
					string s = xml_operations::getAttVal(def_cat, N);
					(*catsApplied)[x].push_back(s);
				}
			}
		}
	}

}

void RuleParser::matchRules(
		map<xmlNode*, vector<pair<unsigned, unsigned> > >* rulesApplied,
		vector<string> slTokens, map<unsigned, vector<string> > catsApplied,
		xmlNode* transfer) {

	xmlNode* section_rules = xml_operations::getChild(transfer, SECTION_RULES);

	vector<unsigned> tokensApplied;

	for (xmlNode* rule = xml_operations::getFirstChild(section_rules); rule; rule =
			xml_operations::getFirstNext(rule)) {

		xmlNode* pattern = xml_operations::getChild(rule, PATTERN);

		// Put pattern items in vector for ease in processing
		vector<xmlNode*> pattern_items;
		for (xmlNode* pattern_item = pattern->children; pattern_item;
				pattern_item = pattern_item->next) {

			pattern_items.push_back(pattern_item);
		}

		for (unsigned i = 0;
				(slTokens.size() >= pattern_items.size())
						&& i <= slTokens.size() - pattern_items.size(); i++) {

			vector<unsigned> slMatchedTokens;
			for (unsigned j = 0; j < pattern_items.size(); j++) {

				// match cat-item with pattern-item
				string slToken = slTokens[i + j];
				vector<string> cats = catsApplied[i + j];

				for (unsigned k = 0; k < cats.size(); k++) {
					// if cat name equals pattern item name
					if (xml_operations::getAttVal(pattern_items[j], N) == cats[k]) {
						slMatchedTokens.push_back(i + j);
						break;
					}
				}
			}
			// if matched tokens' size = pattern items' size
			// then this rule is matched
			if (slMatchedTokens.size() == pattern_items.size()) {
				if (slMatchedTokens.size() == 1)
					tokensApplied.insert(tokensApplied.end(),
							slMatchedTokens.begin(), slMatchedTokens.end());
				(*rulesApplied)[rule].push_back(
						pair<unsigned, unsigned>(slMatchedTokens[0],
								slMatchedTokens.size()));
			}

		}

	}

	// set a default rule for tokens without rules applied
	vector<pair<unsigned, unsigned> > tokensNotApp;
	for (unsigned i = 0; i < slTokens.size(); i++) {
		bool found = false;
		for (unsigned j = 0; j < tokensApplied.size(); j++) {
			if (i == tokensApplied[j]) {
				found = true;
				break;
			}
		}
		if (!found) {
//	  vector<unsigned> tokenNotApp;
//	  tokenNotApp.push_back (i);
//	  tokensNotApp.push_back (tokenNotApp);
			tokensNotApp.push_back(pair<unsigned, unsigned>(i, 1));
		}
	}

	xmlNode* defaultRule;

	(*rulesApplied)[defaultRule] = tokensNotApp;
}

// to sort attribute tags descendingly
bool sortParameter(vector<string> a, vector<string> b) {
	return (a.size() > b.size());
}

map<string, vector<vector<string> > > RuleParser::getAttrs(xmlNode* transfer) {
	map<string, vector<vector<string> > > attrs;
	xmlNode* section_def_attrs = xml_operations::getChild(transfer, SECTION_DEF_ATTRS);

	for (xmlNode* def_attr = xml_operations::getFirstChild(section_def_attrs); def_attr;
			def_attr = xml_operations::getFirstNext(def_attr)) {

		vector<vector<string> > allTags;
		for (xmlNode* attr_item = xml_operations::getFirstChild(def_attr); attr_item;
				attr_item = xml_operations::getFirstNext(attr_item)) {

			// splitting tags by '.'
			string tagsString = xml_operations::getAttVal(attr_item, TAGS);
			char tagsChars[tagsString.size()];
			strcpy(tagsChars, tagsString.c_str());

			vector<string> tags;

			char * tag;
			tag = strtok(tagsChars, ".");
			while (tag != NULL) {
				tags.push_back(tag);
				tag = strtok(NULL, ".");
			}

			allTags.push_back(tags);
		}
		// sort the tags , descendingly by their size
		sort(allTags.begin(), allTags.end(), sortParameter);
//      cout << def_attr.attribute (N).value () << endl;
		attrs[xml_operations::getAttVal(def_attr, N)] = allTags;
	}

	return attrs;
}

map<string, string> RuleParser::getVars(xmlNode* transfer) {
	map<string, string> vars;

	xmlNode* section_def_vars = xml_operations::getChild(transfer, SECTION_DEF_VARS);
	if (section_def_vars)
		for (xmlNode* def_var = xml_operations::getFirstChild(section_def_vars); def_var;
				def_var = xml_operations::getFirstNext(def_var)) {

			vars[xml_operations::getAttVal(def_var, N)] = xml_operations::getAttVal(def_var, V);
		}

	return vars;
}

map<string, vector<string> > RuleParser::getLists(xmlNode* transfer) {
	map<string, vector<string> > lists;

	xmlNode* section_def_lists = xml_operations::getChild(transfer, SECTION_DEF_LISTS);
	if (section_def_lists)
		for (xmlNode* def_list = xml_operations::getFirstChild(section_def_lists); def_list;
				def_list = xml_operations::getFirstNext(def_list)) {

			vector<string> list;
			for (xmlNode* list_item = xml_operations::getFirstChild(def_list); list_item;
					list_item = xml_operations::getFirstNext(list_item)) {

				list.push_back(xml_operations::getAttVal(list_item, V));
			}
			lists[xml_operations::getAttVal(def_list, N)] = list;
		}

	return lists;
}
