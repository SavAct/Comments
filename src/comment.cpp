#include <comment.hpp>

ACTION comment::clearentry(string& link) {

  // Get rLink and strScope
  string rLink;
  string strScope = getDomainScope(link, rLink);

  // Get scope value
  uint64_t scopeValue = scopeToValue(strScope);

  // Get table by scope
  section_table _section(get_self(), scopeValue);

  // Get the primary key
  uint64_t primary = getPrimaryKey(rLink);

  // Delete an entry by rLink
  auto itr = _section.find(primary);
  if(itr != _section.end()) {

    auto entryItr = findEntry(itr, rLink);  // Find entry
    if(entryItr != itr->entries.end()){
      require_auth(entryItr->creator);      // Check creators authority
      
      // Remove this entry
      _section.modify(itr, get_self(), [&](auto& tbLine) {
        tbLine.entries.erase(entryItr);
      });

      // Delete the whole table line if there are no entries left
      if(itr->entries.empty()) {
        itr = _section.erase(itr);					
      }
    } else {
      // Delete the table line if there are no entries 
      itr = _section.erase(itr);
    }
  }
}

std::string comment::getDomainScope(const string& link, string& rLink){
  auto cstr = link.c_str();

  // Find the start of the string
  int start = 0;
  if(strncmp(cstr, "http://", 7) == 0){
    start = 7;
  } else if (strncmp(cstr, "https://", 8) == 0){
    start = 8;
  }
  auto cstr2 = &cstr[start];
  if(strncmp(cstr2, "www.", 4) == 0){
    start += 4;
    cstr2 = &cstr2[4];
  }
  if(strncmp(cstr2, "m.", 2) == 0){
    start += 2;
  }

  // Find the end of the domain
  int end = link.length();
  for(char* c = ((char*)cstr) + start; c < cstr + end; c++){
    switch(*c){
      case '/':
      case '#':
      case '?':
      case '&':
      end = c - cstr;
      break;
    }
  }
  
  check(end > start, "No valid link.");

  // Use only the first 12 characters
  int length = end - start;
  if(length > 12){
    length = 12;
  }

  // Get the scope and rLink as strings
  rLink = link.substr(start + length);
  return link.substr(start, length);  
}

uint64_t comment::scopeToValue(const string& scope){

  uint64_t value;
  char v(0x00);
  int power(0), trip(0);
  uint16_t triplet[4] = {0, 0, 0, 0};  // The uint64_t value will be seperated into four uint16_t parts

  for(const char& c : scope) {

    // Convert the chars of "abcdefghijklmnopqrstuvwxyz-.0123456789:" to a number between 1 and 39
    char cl = std::tolower(c);
    if(cl <= 'z' && cl >= 'a'){
      v = cl - 'a' + 1; 
    } else if(cl >= '0' && cl <= ':'){
      v = cl - '0' + 29;
    } else if(cl == '-'){
      v = 27;
    } else if(cl == '.'){
      v = 28;
    }else {
      check(false, "Invalid character in domain.");
    }

    // Calculate the power
    uint16_t pos;
    switch(power){
      case 0: pos = 1;
      power = 1;
      break;
      case 1: pos = 40;
      power = 2;
      break;
      case 2: pos = (40 * 40);
      power = 3;
      break;
    }

    // Sum all up to a maximum of 64000
    triplet[trip] += v * pos;

    if(power == 3){
    	trip++;
    	power = 0;
	  }
  }

  // Combine them to a single uint64_t
  return triplet[0] | (uint64_t)triplet[1] << 16 | (uint64_t)triplet[2] << 32 | (uint64_t)triplet[3] << 48;
}

uint64_t comment::getPrimaryKey(string& rLink){
  eosio::checksum160 fullhash = eosio::sha1(rLink.c_str(), rLink.length());
  uint64_t primary;
  memcpy(&primary, fullhash.data(), 8);
  return primary;
}

std::list<comment::entry>::const_iterator comment::findEntry(comment::section_table::const_iterator itr, name creator){
	auto entryItr = itr->entries.begin();
	while (entryItr != itr->entries.end()) {
    if(entryItr->creator == creator){
			break;
		}
		++entryItr;
	}
	return entryItr;
}	

std::list<comment::entry>::const_iterator comment::findEntry(comment::section_table::const_iterator itr, string& rLink){
	auto entryItr = itr->entries.begin();
	while (entryItr != itr->entries.end()) {
    if(std::equal(entryItr->rLink.begin(), entryItr->rLink.end(), std::begin(rLink))){
			break;
		}
		++entryItr;
	}
	return entryItr;
}