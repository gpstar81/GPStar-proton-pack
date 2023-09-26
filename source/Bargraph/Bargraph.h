void setupBargraph() {
  WIRE.begin();

  byte by_error, by_address;
  unsigned int i_i2c_devices = 0;

  // Scan i2c for any devices (28 segment bargraph).
  for(by_address = 1; by_address < 127; by_address++ ) {
    WIRE.beginTransmission(by_address);
    by_error = WIRE.endTransmission();

    if(by_error == 0) {
      i_i2c_devices++;
    }
  }

  if(i_i2c_devices > 0) {
    b_bargraph_present = true;
  }
  else {
    b_bargraph_present = false;
  }

  if(b_bargraph_present) {
    ht_bargraph.begin(0x00);
  }
}

void bargraphSetElement(int i_element) {
  if(i_element < 0) {
    i_element = 0; // Keep value positive. 
  }
  if(i_element >= i_bargraph_elements) {
    // Do not exceed the addressable elements.
    i_element = i_bargraph_elements - 1;
  }

  ht_bargraph.setLedNow(i_bargraph[i_element]);
}

void bargraphFull() {
  if(b_bargraph_present) {
    for(uint8_t i = 0; i < i_bargraph_elements; i++) {
      ht_bargraph.setLedNow(i_bargraph[i]);
    }
  }
}

void bargraphClear() {
  if(b_bargraph_present) {
    ht_bargraph.clearAll();
    ms_bargraph.stop();
  }
}

void bargraphUpdate() {

  
  i_bargraph_element++;

  ms_bargraph.start(d_bargraph_ramp_interval);
}