#ifndef CHAINHEAD_H
#define CHAINHEAD_H

#include <shared_mutex>
#include "block.h"
#include "db.h"
#include "utils.h"

class ChainHead {
  private:
    std::shared_ptr<DBService> &dbServer;
    std::deque<std::shared_ptr<const Block>> internalChainHead;
    std::unordered_map<std::string,std::shared_ptr<const Block>> lookupBlockByHash;
    std::unordered_map<std::string,std::shared_ptr<const Block>> lookupBlockByTxHash;
    std::unordered_map<std::string,std::shared_ptr<const Tx::Base>> lookupTxByHash;
    std::unordered_map<std::string,uint64_t> lookupBlockHeightByHash;
    std::unordered_map<uint64_t,std::string> lookupBlockHashByHeight;
    // Used for cacheing blocks and transactions when they are used to load from a DB.
    // See that functions getBlock returns a reference to a shared_ptr.
    // We need to make sure that the reference exists after the scope of the function.
    // TODO: figure out a way to clean up the shared_ptr after the scope of parent function or after X unused time.
    // shared_ptr::use_count can be used for this.
    mutable std::unordered_map<std::string,std::shared_ptr<const Block>> cachedBlocks;
    mutable std::unordered_map<std::string,std::shared_ptr<const Tx::Base>> cachedTxs;
    // Mutable to provide better const correctness for getBlock and other functions. its use is accaptable for mutexes and cache.
    mutable std::shared_mutex internalChainHeadLock;
    bool hasBlock(std::string const &blockHash) const;
    bool hasBlock(uint64_t const &blockHeight) const;
    // Only access these functions if you are absolute sure that internalChainHeadLock is locked.
    void _push_back(const std::shared_ptr<const Block> &&block);
    void _push_front(const std::shared_ptr<const Block> &&block);

  public:
    ChainHead(std::shared_ptr<DBService> &_dbService) : dbServer(_dbService) {
      this->loadFromDB();
    }
    // Mutex locked.
    // When a block is accepted, it is directly moved to chainHead.
    // Any function calling push_back/front should keep in mind that block will be moved.
    void push_back(const std::shared_ptr<const Block> &&block);
    void push_front(const std::shared_ptr<const Block> &&block);
    void pop_back();
    void pop_front();
    const bool exists(std::string const &blockHash) const;
    const bool exists(uint64_t const &blockHeight) const;
    // chainHead does not return a reference to the pointer
    // Because we need to make sure that the reference exists after the scope of the function.
    const std::shared_ptr<const Block> getBlock(std::string const &blockHash) const;
    const std::shared_ptr<const Block> getBlock(uint64_t const &blockHeight) const;
    bool hasTransaction(const std::string &txHash) const;
    const std::shared_ptr<const Tx::Base> getTransaction(const std::string &txHash) const;
    const std::shared_ptr<const Block> getBlockFromTx(const std::string &txHash) const;
    const std::shared_ptr<const Block> latest() const;
    uint64_t blockSize();
    void loadFromDB();
    void dumpToDB();
    void periodicSaveToDB();  // TODO: implement this, do not forget that blocks are stored as shared_ptr, if you erase the item inside the map, all remaining pointers will be undefined.
};

#endif  // CHAINHEAD_H