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

  BARGRAPH_STATE = BG_UNKNOWN; // Always initialized to an unknown state.
}

void bargraphSetElement(int i_element, bool b_power) {
  if(i_element < 0) {
    i_element = 0; // Keep value in range. 
  }
  if(i_element >= i_bargraph_elements) {
    // Do not exceed the addressable elements.
    i_element = i_bargraph_elements - 1;
  }

  if(b_bargraph_present) {
    if(b_power) {
      ht_bargraph.setLedNow(i_bargraph[i_element]);
    }
    else {
      ht_bargraph.clearLedNow(i_bargraph[i_element]);
    }
  }
}

void bargraphFull() {
  if(b_bargraph_present) {
    for(uint8_t i = 0; i < i_bargraph_elements; i++) {
      bargraphSetElement(i, 1);
    }
    BARGRAPH_STATE = BG_FULL;
  }
}

void bargraphReset() {
  i_bargraph_element = 0;
  BARGRAPH_STATE = BG_EMPTY;
  ms_bargraph.stop();
}

void bargraphClear() {
  if(b_bargraph_present) {
    ht_bargraph.clearAll();
  }
  bargraphReset();
}

void bargraphUpdate() {
  // Perform an update of element(s) when timer has completed.
  if(ms_bargraph.remaining() == 0) {
    switch(BARGRAPH_PATTERN) {
      case BG_RAMP_UP:
        if(BARGRAPH_STATE != BG_EMPTY) {
          // Make sure bargraph is empty before ramp up.
          bargraphClear();
        }

        // Update element only when timer is finished.
        bargraphSetElement(i_bargraph_element, 1);

        // Increment to the next element.
        i_bargraph_element++;

        if(i_bargraph_element >= i_bargraph_elements) {
          // Note that the bargraph is full;
          BARGRAPH_STATE = BG_FULL;

          // Set an extra delay at end of sequence.
          ms_bargraph.start(i_bargraph_delay * 2);
        }
        else {
          // Reset timer for next iteration.
          ms_bargraph.start(i_bargraph_delay);
        }
      break;

      case BG_RAMP_DOWN:
        if(BARGRAPH_STATE != BG_FULL) {
          // Make sure bargraph is full before ramp down.
          bargraphFull();
        }

        // Update element only when timer is finished.
        bargraphSetElement(i_bargraph_element, 0);

        // Decrement to the next element.
        i_bargraph_element--;

        if(i_bargraph_element <= 0) {
          // Make sure bargraph is cleared;
          bargraphClear();

          // Set an extra delay at end of sequence.
          ms_bargraph.start(i_bargraph_delay * 2);
        }
        else {
          // Reset timer for next iteration.
          ms_bargraph.start(i_bargraph_delay);
        }
      break;
    }
  }
}