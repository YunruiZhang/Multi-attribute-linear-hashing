// query.c ... query scan functions
// part of Multi-attribute Linear-hashed Files
// Manage creating and using Query objects


#include "defs.h"
#include "query.h"
#include "reln.h"
#include "tuple.h"

// A suggestion ... you can change however you like

struct QueryRep {
	Reln    rel;       // need to remember Relation info
	Bits    known;     // the hash value from MAH
	Bits    unknown;   // the unknown bits from MAH
	PageID  curpage;   // current page in scan do not change wehn switch to overflow page
	int     is_ovflow; // are we in the overflow pages?
	Offset  curtup;    // offset of current tuple within page
	PageID	overflow_id; // this is the overflow page id 0 if not in ovflow
	Count 	curtup_count; // this is the current No of tuple we are looking at in the page
	char *	qstring; // the original query

};

// take a query string (e.g. "1234,?,abc,?")
// set up a QueryRep object for the scan

Query startQuery(Reln r, char *q)
{
	Count nvals = nattrs(r);
	//check if q is in the right format
	int count = 0;
	for(char *curr = q; *curr != '\0'; ++curr){
		if(*curr == ','){
			++count;
		}
	}
	if(count+1 != nvals){
		return NULL;
	}

	Query new = malloc(sizeof(struct QueryRep));
	assert(new != NULL);
	

	char **vals = malloc(nvals*sizeof(char *));
	assert(vals != NULL);
	tupleVals(q, vals);
	ChVecItem *choice_vector = chvec(r);

	// use tupleHash to hash the q and put the bits generate by known attr in known bits
	Bits hashKey = tupleHash(r, q);

	Bits unknown_bits = 0;
	Bits known_bits = 0;
	for(int i = 0; i < MAXBITS; ++i){
		if(strcmp(vals[choice_vector[i].att], "?") == 0){
			unknown_bits = setBit(unknown_bits, i);
		}else{
			int check = bitIsSet(hashKey, i);
			if(check){
				known_bits = setBit(known_bits, i);
			}
		}
	}

	// get first page id
	PageID first = getLower(known_bits, depth(r));

	new -> rel = r;
	new -> known = known_bits;
	new -> unknown = unknown_bits;
	new -> curpage = first;
	new -> is_ovflow = 0;
	new -> curtup = 0;
	new -> overflow_id = 0;
	new -> curtup_count = 0;
	new -> qstring = q;
	// TODO
	// Partial algorithm:
	// form known bits from known attributes
	// form unknown bits from '?' attributes
	// compute PageID of first page
	// using known bits and first "unknown" value
	// set all values in QueryRep object

	return new;
}

// get next tuple during a scan

Tuple getNextTuple(Query q)
{
	
	// TODO
	// Partial algorithm:
	// if (more tuples in current page)
	//    get next matching tuple from current page
	// else if (current page has overflow)
	//    move to overflow page
	//    grab first matching tuple from page
	// else
	//    move to "next" bucket
	//    grab first matching tuple from data page
	// endif
	// if (current page has no matching tuples)
	//    go to next page (try again)
	// endif 


	start: ;

	Page curpg;
	if(q -> is_ovflow == 1){
		curpg = getPage(ovflowFile(q -> rel) , q -> overflow_id);
	}else{
		curpg = getPage(dataFile(q -> rel) , q -> curpage);
	}
	
	char * pgdata = pageData(curpg); 
	Count t_total = pageNTuples(curpg);
	
		// if there is tuple left in the current page 
	for(Offset cur_offset = q -> curtup; q -> curtup_count < t_total; cur_offset += (strlen(&pgdata[cur_offset])+1), ++q -> curtup_count){
		if(tupleMatch(q -> rel, &pgdata[cur_offset], q -> qstring)){
			++q -> curtup_count;
			q -> curtup = cur_offset + strlen(&pgdata[cur_offset])+1;
			return &pgdata[cur_offset];
		}
	}

	// check if curr page has overflow and if the overflow page has overflow
	while(pageOvflow(curpg) != NO_PAGE){
		q -> is_ovflow = 1;
		q -> curtup = 0;
		q -> overflow_id = pageOvflow(curpg);
		q -> curtup_count = 0;

		goto start;
	}

	// if we gets here it means there is no ovflow for this page and we are at the end of a page
	q -> is_ovflow = 0;
	q -> overflow_id = 0;
	q -> curtup = 0;
	q -> curtup_count = 0;
	// if the curpage is before sp check the depth + 1
	if(q -> curpage < splitp(q -> rel)){
		//if known [depth +1] == 1 or unknow [depth +1] == 1 we need to check the corr depth +1 page
		if(bitIsSet(q -> known, depth(q -> rel)) || bitIsSet(q -> unknown, depth(q -> rel))){
			PageID after_sp = setBit(q -> curpage, depth(q -> rel));
			q -> curpage = after_sp;
			goto start;
		}
		
	}else{
		// need to unset the depth +1 bit so that the grab next bucket works 
		// if cur 2^depth > page > sp  this will do nothing  
		q -> curpage = unsetBit(q -> curpage, depth(q -> rel));
		
	}
	//grab the next bucket
	q -> overflow_id = 0;
	q -> curtup = 0;
	q -> curtup_count = 0;

	for(PageID pid=q->curpage + 1; pid < (1 << depth(q -> rel)); ++pid){
		Bits set_bits = pid ^ q -> known;
		int flag = 0;
		for(int i = 0; i < depth(q -> rel); ++i){
			if(bitIsSet(set_bits, i)){
				if(bitIsSet(q -> unknown, i)){
					continue;
				}else{
					flag = 1;
					break;
				}
			}
		}
			// this bucket does not match out patten
		if(flag == 1){
			continue;
		}else{
			// set cur page to this page and do all the things over again
			q -> curpage = pid;
			goto start;
		}

		
	}

	return NULL;
}

// clean up a QueryRep object and associated data

void closeQuery(Query q)
{
	free(q);
	// TODO
}
