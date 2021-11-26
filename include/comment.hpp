#include <eosio/eosio.hpp>
#include <eosio/crypto.hpp>
#include <eosio/system.hpp>

using namespace std;
using namespace eosio;

CONTRACT comment : public contract {
  public:
    using contract::contract;

    // Reference to the last comment
    struct Ref{
      int64_t bTime;      // Timestamp of the last comment
      checksum256 trxId;  // Transaction Id of the last comment
    };

    struct Entry{
      string rLink;       // Rest of the link which is not part of the scope
      uint64_t like;      // Sum of system tokens as likes
      uint64_t disl;      // Sum of system tokens as dislikes
      name creator;       // User which creates the entry and who is able to delete it
      Ref ref;
    };

    /**
     * @brief Create a new comment section
     * 
     * @param creator RAM payer, recipient of payment amounts and authorized account to free the RAM of this comment section
     * @param link The link to which the comments belong
     */ 
    ACTION section(name creator, string& link);

    /**
     * @brief Add a new comment
     *
     * @param link The link to which the comments belong
     * @param ref Reference to teh last comment 
     * @param msg Comment message
     */ 
    ACTION addcomment(string& link, Ref& ref, string& msg){
      // TODO
    }
    
    /**
     * @brief Add a new comment to another comment
     * 
     * @param link The link to which the comments belong
     * @param ref Reference to the last comment
     * @param cRef Reference to the comment which should be commented
     * @param msg Comment message
     */ 
    ACTION comcomment(string& link, Ref& ref, Ref& cRef, string& msg){
      // TODO
    }

    /**
     * @brief Remove the entry of a link
     * 
     * @param link The link
     */ 
    ACTION clearentry(string& link);

  private:
    // Scope is the first 12 chars of the domain without "www.", "http://", "https://" and "m." is excluded
    TABLE sections {
      uint64_t hash;        // First 8 Bytes of the hash of the rlink
      list<Entry> entries;
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
    static list<Entry>::const_iterator findEntry(section_table::const_iterator itr, name creator);
    /**
     * @brief Find entry by rLink
     * 
     * @param itr iterator of the table line
     * @param creator creator name
     * @return iterator of the entry
     */
    static list<Entry>::const_iterator findEntry(section_table::const_iterator itr, string& rLink);

    /**
     * @brief Get the primary key by the rLink
     * 
     * @param rLink 
     * @return The primary key
     */
    static uint64_t getPrimaryKey(string& rLink);

    /**
     * @brief Get the domain and the rest of a link. 
     * 
     * @param link The link
     * @param rLink Will containing the rest of the link
     * @return Returns the first 12 digits of the domain
     */
    static string getDomainScope(const string& link, string& rLink);

    /**
     * @brief Converts a string to an uint64_t value
     * 
     * @param scope String of maximum 12 characters which are allowed in domain-names
     * @return uint64_t 
     */
    static uint64_t scopeToValue(const string& scope);
};
