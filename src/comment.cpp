#include <comment.hpp>

ACTION comment::section(name creator, string& link){
  require_auth(creator);

  // Get rLink and corresponding table
  string rLink;
  section_table _section(get_self(), scopeToValue(getDomainScope(link, rLink)));

  // Find the table line
  auto itr = _section.find(getPrimaryKey(rLink));

  // Create a new reference
  Ref ref = {
    // Note: .trxId Will be initiated to zero by itself
    .bTime = getTime()
  };

  // create entry object
  Entry newEntry = {
    .rLink = rLink,
    .like = 0,
    .disl = 0,
    .count = 0,
    .creator = creator,
    .ref = ref
  };

  if(itr == _section.end()){
    // Creare a new tbLine and add the entry
    _section.emplace(creator, [&](auto& tbLine) {
      tbLine.hash = getPrimaryKey(rLink);
      tbLine.entries.push_back(newEntry);
    });
  } else {
    auto entryItr = findEntry(*itr, rLink);  // Find entry
    check(entryItr != itr->entries.end(), "This link already exits.");

    // Add entry to this tbLine
    _section.modify(itr, creator, [&](auto& tbLine) {
      tbLine.entries.push_back(newEntry);
    });
  }
}

ACTION comment::clearentry(string& link) {
  // Get rLink and corresponding table
  string rLink;
  section_table _section(get_self(), scopeToValue(getDomainScope(link, rLink)));

  // Get the entry of the link
  auto itr = _section.find(getPrimaryKey(rLink));   // Get the table line
  check(itr != _section.end(), "Comment section doesn't ecxists.");
  auto entryItr = findEntry(*itr, rLink);            // Get the entry

  // Delete an entry
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

std::list<comment::Entry>::iterator comment::findEntry(comment::sections& item, const string& rLink){
	std::list<comment::Entry>::iterator entryItr = std::begin(item.entries);
	while (entryItr != item.entries.end()) {
    if(std::equal(entryItr->rLink.begin(), entryItr->rLink.end(), std::begin(rLink))){
			break;
		}
		++entryItr;
	}
	return entryItr;
}	

std::list<comment::Entry>::const_iterator comment::findEntry(const comment::sections& item, const string& rLink){
	auto entryItr = item.entries.begin();
	while (entryItr != item.entries.end()) {
    if(std::equal(entryItr->rLink.begin(), entryItr->rLink.end(), std::begin(rLink))){
			break;
		}
		++entryItr;
	}
	return entryItr;
}

checksum256 comment::get_trx_id() {
  size_t size = eosio::transaction_size();
  char buf[size];
  size_t read = eosio::read_transaction(buf, size);
  check(size == read, "read_transaction failed");
  return eosio::sha256(buf, read);
}

void comment::updateCommentRef(const string& link, const Ref& ref, const bool directComment){
  // Get rLink and corresponding table
  string rLink;
  section_table _section(get_self(), scopeToValue(getDomainScope(link, rLink)));

  // Get the table line of the link
  auto itr = _section.find(getPrimaryKey(rLink));   // Get the table line
  check(itr != _section.end(), "Comment section doesn't exists.");

  // Edit the entry of the link
  _section.modify(itr, same_payer, [&](auto& tbLine) {
    auto entryItr = findEntry(tbLine, rLink);
    check(entryItr != tbLine.entries.end(), "Comment section doesn't exists.");
    check(ref.bTime == entryItr->ref.bTime, "Block time doesn't match.");
    check(ref.trxId == entryItr->ref.trxId, "Transaction Id doesn't match.");

    entryItr->ref.bTime = getTime();
    entryItr->ref.trxId = get_trx_id();
    if(directComment){
      entryItr->count++;
    }
  });
}