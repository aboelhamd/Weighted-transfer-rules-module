Ambiguous patterns are ones that more than one transfer rule could be applied to.
Apertium resolves this ambiguity by applying the left-to-right longest match (LRLM) rule,
and that is not adequate with all the word/s that follow that pattern/s.
To improve this resolution, a new module was introduced to assign weights to these transfer rules
for the word/s that follow the ambiguous pattern, and this is done by training a corpus to generate
maximum entropy models (models with weighted rules), then these models are used to choose the best
(highest weighted) transfer rule to apply.

The weighted transfer rules module was built to apply only chunker transfer rules (patterns of words).
And this project is to improve it by modify some of the methods used and then to extend it
to be applied to interchunk and postchunk transfer rules (patterns of chunks) too.
