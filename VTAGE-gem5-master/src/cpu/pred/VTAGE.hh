#include <vector>
#include <deque>
#include <math.h>
#include <cstring>
#include <stdio.h>

#ifndef __CPU_PRED_VTAGE_HH__
#define __CPU_PRED_VTAGE_HH__

#include "params/VTAGE.hh"
#include "sim/sim_object.hh"
#include "base/statistics.hh"
#include "base/types.hh"
#include "cpu/inst_seq.hh"
#include "cpu/static_inst.hh"
#include "cpu/pred/valuepred.hh"


#define NUM_TAGGED_TABLES 6
#define ALPHA 2
#define L_1 2
#define BASE_TABLE_SIZE 8192
#define BASE_TABLE_LOG_SIZE 13
#define TAGGED_TABLE_SIZE 1024
#define TAGGED_TABLE_LOG_SIZE 10
#define HISTORY_BUFFER_SIZE 65536 //TODO
#define TAG_WIDTH 12
#define MAX_HIST 64
#define MIN_HIST 2
#define ULL(N) ((uint64_t)N##ULL)

struct instrInfo
{
	uint64_t pc;
	uint8_t piece;
	int baseTableIndex;
	int taggedTableIndex [NUM_TAGGED_TABLES];
	uint16_t tags [NUM_TAGGED_TABLES];
	int provider;
	bool isProviderBaseTable;
	double predictedValue;
	unsigned comp [NUM_TAGGED_TABLES];
	int pathHist;
	bool isPredicted;
};

struct FoldedHistory
{
        unsigned comp;
        int compLength;
        int origLength;
        int outpoint;

        void init(int original_length, int compressed_length)
        {
            comp = 0;
            origLength = original_length;
            compLength = compressed_length;
            outpoint = original_length % compressed_length;
        }

        void update(uint8_t * h)
        {
            comp = (comp << 1) | h[0];
            comp ^= h[origLength] << outpoint;
            comp ^= (comp >> compLength);
            comp &= (ULL(1) << compLength) - 1;
        }
};

struct untaggedEntry
{
	double predictedValue;
	uint64_t confidenceCounter;
	uint64_t inflight;

	untaggedEntry() : predictedValue(0), confidenceCounter(0), inflight(0) {}

	untaggedEntry (const untaggedEntry& other): predictedValue(0), confidenceCounter(0), inflight(0) {} 
 };

struct taggedEntry
{
	uint16_t partialTag;
	uint64_t usefulCounter;
	double predictedValue;
	uint64_t confidenceCounter;
	uint64_t inflight;

	taggedEntry() : partialTag(0), usefulCounter(0), predictedValue(0), confidenceCounter(0), inflight(0) {}
	taggedEntry(const taggedEntry& other) : partialTag(0), usefulCounter(0), predictedValue(0), confidenceCounter(0), inflight(0) {}
};

struct threadHistory
{
	int pathHist;
	uint8_t *globalHistory;
	uint8_t *gHist;
	int ptGhist;
	int histBufferSize;
	int minHist;
	int maxHist;
	FoldedHistory *computeIndices;
	FoldedHistory *computeTags[2];

	threadHistory() : pathHist(0)
	{
	        globalHistory = new uint8_t[HISTORY_BUFFER_SIZE];
        	gHist = globalHistory;
        	memset(gHist, 0, histBufferSize);
        	ptGhist = 0;
		histBufferSize = HISTORY_BUFFER_SIZE;
        	computeIndices = new FoldedHistory[NUM_TAGGED_TABLES + 1];
        	computeTags[0] = new FoldedHistory[NUM_TAGGED_TABLES + 1];
       		computeTags[1] = new FoldedHistory[NUM_TAGGED_TABLES + 1];
		minHist = MIN_HIST;
		maxHist = MAX_HIST;
	} 
};

struct inflightInfo
{
	uint64_t seqNum;
	uint64_t tableNum;
	uint64_t index;

	inflightInfo (uint64_t sn, uint64_t tn, uint64_t indx) : seqNum(sn), tableNum(tn), index(indx) {}
};

struct baseTable_t
{
	std::vector<untaggedEntry> entries;
	int mask;

	baseTable_t ()
	{
		entries.resize(BASE_TABLE_SIZE);
		mask = BASE_TABLE_SIZE - 1;
	}
};

struct taggedTable_t
{
	std::vector<taggedEntry> entries;
	int histLength;
	int tagWidth;
	int tagTableSize; //log of num of entries
	int mask;

	taggedTable_t()
	{
		entries.resize(TAGGED_TABLE_SIZE);
		tagTableSize = TAGGED_TABLE_LOG_SIZE;
		tagWidth = TAG_WIDTH;
		mask = TAGGED_TABLE_SIZE - 1;
	}

};

class VTAGE : public ValuePredictor
{

	baseTable_t baseTable;
	taggedTable_t taggedTables[NUM_TAGGED_TABLES];
  	std::deque<inflightInfo> inflightPreds;	
	threadHistory history;

	uint64_t F(uint16_t A, int size, int bank) const;
	int computeBaseTableIndex (instrInfo &instr);
	int computeTaggedTableIndex(instrInfo &info, int bank) const;
	uint16_t computeTag (uint64_t pc, uint8_t piece, int bank) const;
	void updateGHist(uint64_t * &h, bool dir, uint64_t * tab, int &pt);
	bool getBaseTablePrediction (instrInfo &instr, uint64_t &confidence);

	bool getTaggedTablePrediction (instrInfo& instr, int bank, uint64_t &confidence);

	public:
	
	VTAGE(const VTAGEParams *params) ;
	bool getPrediction(uint64_t seq_no, uint64_t pc, uint8_t piece, uint8_t* bHist, int pHist, double& predicted_value);
	void updateValuePredictor(uint64_t seq_no, uint8_t piece, double actual_value);

	private:	
	
	void updateHistories (uint8_t* gHist, int pHist);
	instrInfo getPrediction (instrInfo instr);
	void updatePredictor (instrInfo instr, double actualValue);

	void printBaseTableEntries ();
	void printTaggedTableEntries (int bank);
	void printIndexAndTag (instrInfo instr);
	void printPrediction (instrInfo instr);
	void computeIndexAndTag (instrInfo &instr);
	
};

#endif













