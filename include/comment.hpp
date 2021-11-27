#include <eosio/eosio.hpp>
#include <eosio/crypto.hpp>
#include <eosio/system.hpp>
#include <eosio/transaction.hpp>
#include <eosio/asset.hpp>

using namespace std;
using namespace eosio;

CONTRACT comment : public contract {
  public:
    using contract::contract;

    // Reference to a transaction of a comment
    struct Ref{
      int64_t bTime;      // Timestamp of the last comment
      checksum256 trxId;  // Transaction Id of the last comment
    };

    struct Entry{
      string rLink;       // Rest of the link which is not part of the scope
      uint64_t like;      // Sum of system tokens as likes
      uint64_t disl;      // Sum of system tokens as dislikes
      uint32_t count;     // Amount of comments, without comment on comments 
      name creator;       // User which creates the entry and who is able to delete it
      Ref ref;            // Reference to the current last comment 
      Ref tRef;           // Reference to the current last top comment (This is no comment to a comment)
    };

    constexpr static name Tokenowners = name("stake.savact");

    /**
     * @brief Notification for system token transfer
     * 
     * @param from Sender
     * @param to Recipient
     * @param fund Asset
     * @param memo The memo begins with '0' for dislikes and '1' for likes. Afterwards it follows ' ' and the link
     */
    [[eosio::on_notify("eosio.token::transfer")]]
    void deposit(name from, name to, asset fund, string memo);

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
      updateCommentRef(link, ref);
    }
    
    /**
     * @brief Add a new comment to another comment
     * 
     * @param link The link to which the comments belong
     * @param ref Reference to the current last comment
     * @param cRef Reference to the comment which should be a top commented (If not this comment will be ignored by client)
     * @param msg Comment message
     */ 
    ACTION comcomment(string& link, Ref& ref, Ref& cRef, string& msg){
      updateCommentRef(link, ref, false);
    }

    /**
     * @brief Remove the entry of a link
     * 
     * @param link The link to which the comments belong
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
     * @brief Send system token to a receiver account
     * 
     * @param to Receiver account name
     * @param funds Asset of system token
     * @param memo Memo for the transfer
     */
    void transfer(name to, asset funds, const std::string& memo);

    /**
     * @brief Find entry by rLink
     * 
     * @param item Item of the table line
     * @param rLink Contains the rest of a link for search
     * @return Iterator of the entry
     */
    static std::list<comment::Entry>::iterator findEntry(comment::sections& item, const string& rLink);
    static std::list<comment::Entry>::const_iterator findEntry(const comment::sections& item, const string& rLink);

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
     * @param link The link to which the comments belong
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

    /**
     * @brief Get the trx id
     * 
     * @return checksum256 
     */
    static checksum256 get_trx_id();

    /**
     * @brief Get the time of the current block in microseconds
     * 
     */
    static int64_t getTime(){
      return eosio::current_block_time().to_time_point().time_since_epoch().count();
    }

    /**
     * @brief Check the data and update the new current last comment 
     * 
     * @param link The link to which the comments belong
     * @param ref Reference to the last comment 
     * @param directComment Defines if this comment is a direct comment (no comment to a comment)
     */
    void updateCommentRef(const string& link, const Ref& ref, const bool directComment = true);
};
