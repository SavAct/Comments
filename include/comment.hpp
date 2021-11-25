#include <eosio/eosio.hpp>
#include <eosio/crypto.hpp>

using namespace std;
using namespace eosio;

CONTRACT comment : public contract {
  public:
    using contract::contract;

    // Clear a single entry
    ACTION clearentry(string& link);

  struct entry{
    string rLink;       // Rest of the link which is not part of the scope
    uint64_t like;      // Sum of system tokens as likes
    uint64_t disl;      // Sum of system tokens as dislikes
    name creator;       // User which creates the entry and who is able to delete it
    uint32_t TS;        // Timestamp of the last comment
    checksum256 TrxId;  // Transaction Id of the last comment
  };

  private:
    // Scope is the first 12 chars of the domain without "www.", "http://", "https://" and "m." is excluded
    TABLE sections {
      uint64_t hash;        // First 8 Bytes of the hash of the rlink
      list<entry> entries;
      auto primary_key() const { return hash; }
    };
    typedef multi_index<name("sections"), sections> section_table;

    /**
     * @brief Find entry by creator name
     * 
     * @param itr iterator of the table line
     * @param creator creator name
     * @return iterator of the entry
     */
    list<entry>::const_iterator findEntry(section_table::const_iterator itr, name creator);
    /**
     * @brief Find entry by rLink
     * 
     * @param itr iterator of the table line
     * @param creator creator name
     * @return iterator of the entry
     */
    list<entry>::const_iterator findEntry(section_table::const_iterator itr, string& rLink);

    /**
     * @brief Get the primary key by the rLink
     * 
     * @param rLink 
     * @return The primary key
     */
    uint64_t getPrimaryKey(string& rLink);

    /**
     * @brief Get the domain and the rest of a link. 
     * 
     * @param link The link
     * @param rLink Will containing the rest of the link
     * @return Returns the first 12 digits of the domain
     */
    string getDomainScope(const string& link, string& rLink);

    /**
     * @brief Converts a string to an uint64_t value
     * 
     * @param scope String of maximum 12 characters which are allowed in domain-names
     * @return uint64_t 
     */
    uint64_t scopeToValue(const string& scope);
};
