#include "AddressGraph.hpp"

// Friend of AddressGraph
class EepromAddressGraph {
private:
  AddressGraph* addressGraph;
  uint8_t *eepromBuffer;
  uint8_t eepromBufferCapacity;

public:
  EepromAddressGraph(AddressGraph* const new_addressGraph, uint8_t* const new_eepromBuffer, const uint8_t new_eepromBufferCapacity)
  : addressGraph(new_addressGraph), eepromBuffer(new_eepromBuffer), eepromBufferCapacity(new_eepromBufferCapacity)
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
    if((edgesToCopy * (uint8_t) sizeof(AddressFilter)) > (eepromBufferCapacity - 1))
      edgesToCopy = (eepromBufferCapacity - 1) / sizeof(AddressFilter);

    DEBUGprint_EEP("EepAG: SvState, edg2Cp: %d\n", edgesToCopy);

  // Anything to copy?
    if(edgesToCopy == 0) return;

  // Write out edge count
    uint8_t* eeprom_ptr = eepromBuffer;
    eeprom_write_byte(eeprom_ptr, edgesToCopy);
    eeprom_ptr++;

  // Write out each edge
    AddressFilter *filter = addressGraph->addressEdges.front();
    for(; edgesToCopy > 0; edgesToCopy--){
    // Just to be cautious
      if(filter >= addressGraph->addressEdges.back()) break;

    // Only active edges are to be saved.
      if(filter->mode != AddressFilter::Mode__Inactive){
    // Write out edge to eeprom
        eeprom_write_block(filter, eeprom_ptr, sizeof(AddressFilter));
    // Move src and dst pointers forward
        eeprom_ptr += sizeof(AddressFilter);
      }

      filter++;
    } 
  }

  void loadState(){
    uint8_t edgesToCopy;

  // Read edge count
    uint8_t* eeprom_ptr = eepromBuffer;
    edgesToCopy = eeprom_read_byte(eeprom_ptr);
    eeprom_ptr++;

    DEBUGprint_EEP("EepAG: LdState, edg2Cp: %d\n", edgesToCopy);

  // Wipe out existing edges
    addressGraph->addressEdges.set_size(0);
  // Preset available capacity, to avoid reallocating repeatedly.
//    if(addressGraph->addressEdges.get_availableCapacity() < edgesToCopy)
    addressGraph->addressEdges.set_availableCapacity(edgesToCopy);

  // Read each edge
    for(; edgesToCopy > 0; edgesToCopy--){
  // Read edge
      AddressFilter filter;
      eeprom_read_block(&filter, eeprom_ptr, sizeof(AddressFilter));

      DEBUGprint_EEP("", filter.debugPrintValues());
  // Address graph will perform its own capacity checks, rejecting the filter if necessary.
      addressGraph->sinkEdge(filter);

    // Move src pointer forward
      eeprom_ptr += sizeof(AddressFilter);
    }
  }
};

