#ifndef FloatKeyContainer_H
#define FloatKeyContainer_H 

#include <EEPROM.h>
typedef unsigned int size_t;
typedef uint8_t byte;

/*
 * Class for store values in format like {double-key-1: value-1, double-key-2:value-2, double-key-3:value-3}
 * Parameters:
 * size - maximal size
 * key_trim_cooficient - cooficient for convertion key to key_type (e.g. if key between 0.0 and 1.0, and type of key is byte - you can use 255)
 * key_type - type of key storage (I use integer types, because used controller have no FPU)
 * value_type - type of stored value
 */
template<size_t size, int key_trim_cooficient, typename key_type, typename value_type> class FloatKeyContainer {
  private:
    size_t elements;
    key_type keys[size];
    value_type values[size];
  public:
    /*
     * Creating new container andd fill it by zeros
     */
    FloatKeyContainer() {
      for(size_t i=0;i<size;i++) {
        this->keys[i] = 0.0;
        this->values[i] = 0;
      }
      this->elements = 0;
    }
    
    /* 
     * Convert double value of key to used type
     */ 
    key_type key_converted(double key) {
      return (key_type)(key*key_trim_cooficient);
    }
    
    /*
     * Insert new value to container
     */
    void insert(double dkey, value_type value) {
      key_type key = this->key_converted(dkey);
      if(this->elements==0) { //We needn't search insert position, if we have no elements
        //Inserting new element to need place
        this->keys[0] = key;
        this->values[0] = value;
        this->elements = 1;
      }
      else { //If have - we need to keep this->keys array sorted, and store "connections" between elements of this->keys and this->values
        size_t i;
        for(i=0;i<this->elements;i++) { //Searching for insert position
          if(this->keys[i]>=key) {
            break;
          }
        }
        for(size_t j=i; j<size-1; j++) { //Moving next elements to right
          this->keys[j+1] = this->keys[j];
          this->values[j+1] = this->values[j];
        }
        //Inserting new element to need place
        this->elements++;
        this->keys[i] = key;
        this->values[i] = value;
      }
    }
    
    /*
     * Get key with neares number (compare with 2 existsing)
     * key - (CONVERTED) key for comparsion with two next
     * first - index of element in this->keys;
     * second - index of element if this->keys
     *
     * E.g. you have next :
     * this->keys = {0, 50}
     * key = 65
     * first = 0, second = 1;
     * It'll return 1;
     */
    inline size_t getNearestKey(key_type key, size_t first, size_t second) {
      return abs(key-this->keys[first]) < abs(this->keys[second]-key) ? first : second;
    }
    
    /*
     * Get key with nearest number (compare total this->keys)
     * dkey - key for comparsion
     * Return index for this->keys and this->values;
     */
    size_t getIndex(double dkey) {
      key_type key = this->key_converted(dkey);
      if(this->elements==1) {
        return 0;
      }
      else {
        //Binary search
        size_t left = 0,
               right = this->elements,
               middle;
        while(left<=right) {
          middle = left+(right-left)/2;
          if(key<this->keys[middle]) {
            right = middle-1;
          }
          else if(key>this->keys[middle]) {
            left = middle+1;
          }
          else {
            break;
          }
        }
        
        //Compare found position and 2 nearly
        if( (middle!=0 && middle!=elements-1) && this->elements>2) {
          return this->getNearestKey(key,
            this->getNearestKey(key, middle-1, middle),
            this->getNearestKey(key, middle, middle+1)
          );
        }
        else if(middle==0) {
          if(elements<2) {
            return 0;
          }
          else {
            return this->getNearestKey(key, 0, 1);
          }
        }
        else if(middle==elements-1) {
          if(size<2) {
            return 0;
          }
          else {
            return this->getNearestKey(key, 0, 1);
          }
        }
      }
    }
    
    /*
     * Get value for key
     */
    value_type get(double key) {
      return this->values[this->getIndex(key)];
    }
    
    /*
     * Set value for key
     */
    void set(double key, value_type value) {
      this->values[this->getIndex(key)] = value;
    }
    
    /*
     * Load container from EEPROM
     */
    void load(size_t address) {
      //Read inserted element count
      byte* elements_count_memory = (byte*)&this->elements; 
      size_t elements_count_memory_size = sizeof(size_t);
      for(size_t i=0; i<elements_count_memory_size; i++) {
        elements_count_memory[i] = EEPROM.read(address + i);
      }
      
      //Read keys
      byte* keys_memory = (byte*)this->keys;
      size_t keys_memory_size = sizeof(key_type)*size;
      for(size_t i=0; i<keys_memory_size; i++) {
        keys_memory[i] = EEPROM.read(address + elements_count_memory_size + i);
      }

      //Read values
      byte* values_memory = (byte*)this->values;
      size_t values_memory_size = sizeof(value_type)*size;
      for(size_t i=0; i<values_memory_size; i++) {
        values_memory[i] = EEPROM.read(address + elements_count_memory_size + keys_memory_size + i);
      }
    }
    
    /*
     * Save container to EEPROM
     */
    void save(size_t address) {
      //Save inserted element count
      byte* elements_count_memory = (byte*)&this->elements;
      size_t elements_count_memory_size = sizeof(size_t);
      for(size_t i=0; i<elements_count_memory_size; i++) {
        EEPROM.write(address + i, elements_count_memory[i]);
      }
      
      //Save keys
      byte* keys_memory = (byte*)this->keys;
      size_t keys_memory_size = sizeof(key_type)*size;
      for(size_t i=0; i<keys_memory_size; i++) {
        EEPROM.write(address + elements_count_memory_size + i, keys_memory[i]);
      }

      //Save values
      byte* values_memory = (byte*)this->values;
      size_t values_memory_size = sizeof(value_type)*size;
      for(size_t i=0; i<values_memory_size; i++) {
        EEPROM.write(address + elements_count_memory_size + keys_memory_size + i, values_memory[i]);
      }
    }
};

#endif
