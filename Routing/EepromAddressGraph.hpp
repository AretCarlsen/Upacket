// Copyright (C) 2010, Aret N Carlsen (aretcarlsen@autonomoustools.com).
// Dynamic routing handlers (C++).
// Licensed under GPLv3 and later versions. See license.txt or <http://www.gnu.org/licenses/>.


#ifndef DEBUGprint_EEP
#define DEBUGprint_EEP(...)
#endif
#ifndef DEBUG_EEP
#define DEBUG_EEP(...)
#endif


#include "AddressGraph.hpp"

// Friend of AddressGraph
class EepromAddressGraph {
private:
  AddressGraph* addressGraph;
  uint8_t *eeprom_edgeCount;
  AddressFilter *eeprom_filters;
  uint8_t eepromBufferCapacity;

public:
  EepromAddressGraph(AddressGraph* const new_addressGraph, uint8_t* new_eeprom_edgeCount,
                     AddressFilter* const new_eeprom_filters, const uint8_t new_eepromBufferCapacity)
  : addressGraph(new_addressGraph), eeprom_edgeCount(new_eeprom_edgeCount),
    eeprom_filters(new_eeprom_filters), eepromBufferCapacity(new_eepromBufferCapacity)
  {
  // Need at least the edge count byte
    assert(eepromBufferCapacity > 0);
    loadState();
  }

  void saveState(){
  // Check how many edges need to be saved.
    uint8_t edgesToCopy = 0;
    // Only active edges will be saved.
    for(AddressFilter *filter = addressGraph->addressEdges.front(); filter < addressGraph->addressEdges.back(); filter++){
      if(filter->mode != AddressFilter::Mode__Inactive)
        edgesToCopy++;
    }

  // Make sure Eeprom has enough capacity for those edges.
  // If not, cut the edge count down the maximum storable (trimming off latest edges).
    if(edgesToCopy > eepromBufferCapacity)
      edgesToCopy = eepromBufferCapacity;

    DEBUGprint_EEP("EepAG: SvState, edg2Cp: %d\n", edgesToCopy);

  // Anything to copy?
    if(edgesToCopy == 0) return;

  // Write out edge count
    eeprom_write_byte(eeprom_edgeCount, edgesToCopy);

  // Write out each edge
    AddressFilter *filter = addressGraph->addressEdges.front();
    AddressFilter *eeprom_filter = eeprom_filters;
    for(; edgesToCopy > 0; edgesToCopy--){
    // Just to be cautious
      if(filter >= addressGraph->addressEdges.back()) break;

    // Only active edges are to be saved.
      if(filter->mode != AddressFilter::Mode__Inactive){
    // Write out edge to eeprom
        eeprom_write_block(filter, eeprom_filter, sizeof(AddressFilter));
    // Move dest pointer forward
        eeprom_filter++;
      }

    // Move source pointer forward
      filter++;
    } 
  }

  void loadState(){
    uint8_t edgesToCopy;

  // Read edge count
    edgesToCopy = eeprom_read_byte(eeprom_edgeCount);

    DEBUGprint_EEP("EepAG: LdState, edg2Cp: %d\n", edgesToCopy);

  // Wipe out existing edges
    addressGraph->addressEdges.set_size(0);
  // Preset available capacity, to avoid reallocating repeatedly.
//    if(addressGraph->addressEdges.get_availableCapacity() < edgesToCopy)
    addressGraph->addressEdges.set_availableCapacity(edgesToCopy);

  // Read each edge
    AddressFilter *eeprom_filter = eeprom_filters;
    for(; edgesToCopy > 0; edgesToCopy--){
  // Read edge
      AddressFilter filter;
      eeprom_read_block(&filter, eeprom_filter, sizeof(AddressFilter));

      DEBUG_EEP(filter.debugPrintValues());
  // Address graph will perform its own capacity checks, rejecting the filter if necessary.
      addressGraph->sinkEdge(filter);

    // Move src pointer forward
      eeprom_filter++;
    }
  }
};

