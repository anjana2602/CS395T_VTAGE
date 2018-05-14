#include <cinttypes>
#include "cvp.h"
#include "vtage.h"
#include <cassert>
#include <cstdio>
#include <cmath>
#include <stdlib.h>
#include <time.h>
#include <map>

static MyVTAGEPredictor vtage;

static std::map <uint64_t, instrInfo> instrList;

uint64_t count = 0;

bool getPrediction(uint64_t seq_no, uint64_t pc, uint8_t piece, uint64_t& predicted_value) {

  
	instrInfo instr;
	instr.pc = pc;
	instr.piece = piece;

	vtage.computeIndexAndTag(instr);

	instrInfo instr1 = vtage.getPrediction (instr);
	instrList.insert(std::pair<uint64_t, instrInfo> (seq_no, instr1));

	predicted_value = instr1.predictedValue;

	return instr1.isPredicted;

}

void speculativeUpdate(uint64_t seq_no,    		// dynamic micro-instruction # (starts at 0 and increments indefinitely)
                       bool eligible,			// true: instruction is eligible for value prediction. false: not eligible.
		       uint8_t prediction_result,	// 0: incorrect, 1: correct, 2: unknown (not revealed)
		       // Note: can assemble local and global branch history using pc, next_pc, and insn.
		       uint64_t pc,			
		       uint64_t next_pc,
		       InstClass insn,
		       uint8_t piece,
		       // Note: up to 3 logical source register specifiers, up to 1 logical destination register specifier.
		       // 0xdeadbeef means that logical register does not exist.
		       // May use this information to reconstruct architectural register file state (using log. reg. and value at updatePredictor()).
		       uint64_t src1,
		       uint64_t src2,
		       uint64_t src3,
		       uint64_t dst) {
	vtage.updateHistories (pc, next_pc, (insn == condBranchInstClass), (insn == uncondIndirectBranchInstClass || insn == uncondDirectBranchInstClass));
	
}

void updatePredictor(uint64_t seq_no,		// dynamic micro-instruction #
		     uint64_t actual_addr,	// load or store address (0xdeadbeef if not a load or store instruction)
		     uint64_t actual_value,	// value of destination register (0xdeadbeef if instr. is not eligible for value prediction)
		     uint64_t actual_latency) {	// actual execution latency of instruction

	if(actual_value == 0xdeadbeef)
	{
		return;
	}
	std::map <uint64_t, instrInfo>::iterator it = instrList.find(seq_no);
	if(it != instrList.end())
	{
		vtage.updatePredictor (it->second, actual_value);
	}

 	
}

void beginPredictor(int argc_other, char **argv_other) {
   if (argc_other > 0)
      printf("CONTESTANT ARGUMENTS:\n");

   for (int i = 0; i < argc_other; i++)
      printf("\targv_other[%d] = %s\n", i, argv_other[i]);

	//vtage.printTaggedTableEntries(5);
}

void endPredictor() {
	printf("CONTESTANT OUTPUT--------------------------\n");
	printf("--------------------------\n");
	//vtage.printTaggedTableEntries(0);
	printf("COUNT IS %ld\n", count);


}

int random (int min, int max)
{
	static bool first = true;

	if (first)
	{
		srand (time (NULL));
		first = false;
	}

	return min + rand() % (( max + 1) - min);
}

MyVTAGEPredictor::MyVTAGEPredictor()
{
	for (int i = 0 ; i < NUM_TAGGED_TABLES; i++)
	{
		taggedTables[i].histLength = (pow (ALPHA, i)) * L_1;
            	history.computeIndices[i].init(taggedTables[i].histLength, taggedTables[i].tagTableSize);
            	history.computeTags[0][i].init(history.computeIndices[i].origLength, taggedTables[i].tagWidth);
            	history.computeTags[1][i].init(history.computeIndices[i].origLength, taggedTables[i].tagWidth);
	}
}

uint64_t MyVTAGEPredictor::F(uint16_t A, int size, int bank) const
{
	uint64_t A1, A2;

	A = A & ((ULL(1) << size) - 1);
	A1 = (A & ((ULL(1) << taggedTables[bank].tagTableSize) - 1));
	A2 = (A >> taggedTables[bank].tagTableSize);
	A2 = ((A2 << (bank + 1)) & ((ULL(1) << taggedTables[bank].tagTableSize) - 1)) + (A2 >> (taggedTables[bank].tagTableSize - bank + 1));
	A = A1 ^ A2;
	A = ((A << (bank + 1)) & ((ULL(1) << taggedTables[bank].tagTableSize) - 1)) + (A >> (taggedTables[bank].tagTableSize - bank + 1));
	return (A);
}

int MyVTAGEPredictor::computeBaseTableIndex (instrInfo &instr)
{
	int index = ((instr.pc << 2) ^ instr.piece) & baseTable.mask;
	return index;	
}

int MyVTAGEPredictor::computeTaggedTableIndex (instrInfo &instr, int bank) const
{
    uint64_t pc = ((instr.pc << 2) ^ instr.piece);

    int index;
    int hlen = (taggedTables[bank].histLength > 16) ? 16 : taggedTables[bank].histLength;
    index =
        (pc) ^ ((pc) >> ((int) std::abs(taggedTables[bank].tagTableSize - bank + 1) + 1)) ^ history.computeIndices[bank].comp ^ F(history.pathHist, hlen, bank);

	instr.comp[bank] = history.computeIndices[bank].comp;
	instr.pathHist = history.pathHist;
    return (index & ((ULL(1) << (taggedTables[bank].tagTableSize)) - 1));
}

uint16_t MyVTAGEPredictor::computeTag (uint64_t pc, uint8_t piece, int bank) const
{
    pc = ((pc << 2) ^ piece);

    uint16_t tag = (pc) ^ history.computeTags[0][bank].comp
                   ^ (history.computeTags[1][bank].comp << 1);

    return (tag & ((ULL(1) << taggedTables[bank].tagWidth) - 1));
	
}

void MyVTAGEPredictor::computeIndexAndTag (instrInfo &instr)
{
	instr.baseTableIndex = computeBaseTableIndex (instr);
	for (int i = 0; i < NUM_TAGGED_TABLES; i++ )
	{
		instr.taggedTableIndex [i] = computeTaggedTableIndex (instr, i);
		instr.tags [i] = computeTag (instr.pc, instr.piece, i);
	}
}

void MyVTAGEPredictor::printIndexAndTag (instrInfo instr)
{
	printf ("Base Index = 0x%x Tagged Indices = ", instr.baseTableIndex);
	for (int i = 0; i < NUM_TAGGED_TABLES; i++ )
	{
		printf("0x%x ", instr.taggedTableIndex[i]);
	}
	printf("Tags = ");
 	for (int i = 0; i < NUM_TAGGED_TABLES; i++ )
	{
		printf("0x%x ", instr.tags[i]);
	}
	printf("\n");
}

void MyVTAGEPredictor::updateGHist(uint64_t * &h, bool dir, uint64_t * tab, int &pt)
{
    if (pt == 0) {
         // Copy beginning of globalHistoryBuffer to end, such that
         // the last maxHist outcomes are still reachable
         // through pt[0 .. maxHist - 1].
         for (int i = 0; i < history.maxHist; i++)
             tab[history.histBufferSize - history.maxHist + i] = tab[i];
         pt =  history.histBufferSize - history.maxHist;
         h = &tab[pt];
    }
    pt--;
    h--;
    h[0] = (dir) ? 1 : 0;
}

void MyVTAGEPredictor::updateHistories (uint64_t pc, uint64_t next_pc,  bool isCondBr, bool isUncondBr)
{
	if (! (isCondBr || isUncondBr))
		return;

	bool taken = (pc  + 4 != next_pc); //TODO

	//  UPDATE HISTORIES
    	bool pathbit = ((next_pc >> 4) & 1);
    	//update user history
	updateGHist(history.gHist, taken, history.globalHistory, history.ptGhist);
	history.pathHist = (history.pathHist << 1) + pathbit;
	history.pathHist = (history.pathHist & ((ULL(1) << 16) - 1));

	//prepare next index and tag computations for user branchs
	for (int i = 0; i < NUM_TAGGED_TABLES; i++)
	{
		history.computeIndices[i].update(history.gHist);
		history.computeTags[0][i].update(history.gHist);
		history.computeTags[1][i].update(history.gHist);
    	}
}

bool MyVTAGEPredictor::getBaseTablePrediction (instrInfo &instr, uint64_t& confidence)
{
	instr.predictedValue = baseTable.entries[instr.baseTableIndex].predictedValue;
	instr.isProviderBaseTable = true;
	confidence = baseTable.entries[instr.baseTableIndex].confidenceCounter;
	return true;
	
}

bool MyVTAGEPredictor::getTaggedTablePrediction (instrInfo &instr, int bank, uint64_t &confidence)
{
	instr.predictedValue = taggedTables[bank].entries[instr.taggedTableIndex[bank]].predictedValue;

	if(taggedTables[bank].entries[instr.taggedTableIndex[bank]].partialTag == instr.tags[bank])
	{
		instr.provider = bank;
		instr.isProviderBaseTable = false;
		confidence = taggedTables[bank].entries[instr.taggedTableIndex[bank]].confidenceCounter; 
		return true;
	}

	return false;	
}

instrInfo MyVTAGEPredictor::getPrediction (instrInfo instr1)
{
	uint64_t confidence;
	instrInfo instr = instr1;
	for (int i = NUM_TAGGED_TABLES - 1; i >= 0; i --)
	{
		if(getTaggedTablePrediction (instr, i, confidence))
		{
			if (confidence == CONF_COUNTER_MAX)
				instr.isPredicted = true;
			else
				instr.isPredicted = false;
			return instr ;
		}
	}

	getBaseTablePrediction (instr, confidence);
	
	if (confidence == CONF_COUNTER_MAX)
		instr.isPredicted = true;
	else
		instr.isPredicted = false;
	return instr ;
	


}

void MyVTAGEPredictor::printBaseTableEntries ()
{
	printf("****************************BASE TABLE**********************************\n");
	for (int i = 0; i < BASE_TABLE_SIZE; i++)
	{
		//printf("%d Predicted Value 0x%lx Confidence 0x%x\n", i, baseTable.entries[i].predictedValue, baseTable.entries[i].confidenceCounter);	
	}
	
}

void MyVTAGEPredictor::printTaggedTableEntries (int bank)
{

	printf("***************************TAGGED TABLE %d ******************************\n", bank);

	for (int i = 0; i < TAGGED_TABLE_SIZE; i++)
	{
		//printf("%d %d Partial Tag 0x%x Useful 0x%x Predicted Value 0x%lx Confidence 0x%x\n", bank, i, taggedTables[bank].entries[i].partialTag, taggedTables[bank].entries[i].usefulCounter, taggedTables[bank].entries[i].predictedValue, taggedTables[bank].entries[i].confidenceCounter);	
	}

}

void MyVTAGEPredictor::printPrediction (instrInfo instr)
{
	int x = 0;
	if (instr.isProviderBaseTable)
		x = 1;

	int y = 0;
	if (instr.isPredicted)
		y = 1;

	int bank = instr.provider; 

	//printf("PREDICTION: Predicted Value = 0x%lx isProviderBaseTable = %x Provider = 0x%x isPredicted = %x Base Confidence = 0x%x Tagged Confidence = 0x%x\n", instr.predictedValue, x, instr.provider, y, baseTable.entries[instr.baseTableIndex].confidenceCounter, taggedTables[bank].entries[instr.taggedTableIndex[bank]].confidenceCounter);
}

bool MyVTAGEPredictor::updatePredictor (instrInfo instr, uint64_t actualValue)
{
	bool misPrediction = false;

	if (!instr.isProviderBaseTable)
	{
		if(taggedTables[instr.provider].entries[instr.taggedTableIndex[instr.provider]].predictedValue == actualValue)
		{
			taggedTables[instr.provider].entries[instr.taggedTableIndex[instr.provider]].confidenceCounter = std::min((uint64_t)CONF_COUNTER_MAX, taggedTables[instr.provider].entries[instr.taggedTableIndex[instr.provider]].confidenceCounter + 1);
			taggedTables[instr.provider].entries[instr.taggedTableIndex[instr.provider]].usefulCounter = 1;
		}
		else
		{
			misPrediction = true;

			if(taggedTables[instr.provider].entries[instr.taggedTableIndex[instr.provider]].confidenceCounter == 0)
			{
				taggedTables[instr.provider].entries[instr.taggedTableIndex[instr.provider]].predictedValue = actualValue;
			}
			taggedTables[instr.provider].entries[instr.taggedTableIndex[instr.provider]].confidenceCounter = 0;
			taggedTables[instr.provider].entries[instr.taggedTableIndex[instr.provider]].usefulCounter = 0;

		}
	}
	else
	{
		if(baseTable.entries[instr.baseTableIndex].predictedValue == actualValue)
		{
			baseTable.entries[instr.baseTableIndex].confidenceCounter = std::min ((uint64_t)CONF_COUNTER_MAX, baseTable.entries[instr.baseTableIndex].confidenceCounter + 1);
		}
		else
		{
			misPrediction = true;
			if(baseTable.entries[instr.baseTableIndex].confidenceCounter == 0)
			{
				baseTable.entries[instr.baseTableIndex].predictedValue = actualValue;
			}

			baseTable.entries[instr.baseTableIndex].confidenceCounter = 0;
		}
	}

	if(misPrediction)
	{
		int upperBanks[NUM_TAGGED_TABLES];
		int numUpperBanks = 0;
		
		int k;
		int i = instr.provider + 1;
		if(instr.isProviderBaseTable)
			i = 0;

		k = i;

		for ( ; i < NUM_TAGGED_TABLES; i++)
		{
			if (taggedTables[i].entries[instr.taggedTableIndex[i]].usefulCounter == 0)
			{
				upperBanks[numUpperBanks] = i;
				numUpperBanks++;
			}
		}
		
		if (numUpperBanks == 0)
		{
			for (i = k; i < NUM_TAGGED_TABLES; i++)
			{
				taggedTables[i].entries[instr.taggedTableIndex[i]].usefulCounter = 0;
			}
		}
		else
		{
			int randomComponent = random (0, numUpperBanks - 1);
			int bank = upperBanks [randomComponent];

			
			taggedTables[bank].entries[instr.taggedTableIndex[bank]].partialTag = instr.tags[bank];
			taggedTables[bank].entries[instr.taggedTableIndex[bank]].usefulCounter = 0;
			taggedTables[bank].entries[instr.taggedTableIndex[bank]].predictedValue = actualValue;
			taggedTables[bank].entries[instr.taggedTableIndex[bank]].confidenceCounter = 0;

		}

	}

	
}


