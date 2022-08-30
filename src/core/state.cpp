#include "state.h"
#include "chainTip.h"

#if !IS_LOCAL_TESTS
State::State(std::shared_ptr<DBService> &dbServer, std::shared_ptr<VMCommClient> &grpcClient) : grpcClient(grpcClient) {
  this->loadState(dbServer);
}
#else
State::State(std::shared_ptr<DBService> &dbServer) {
  this->loadState(dbServer);
}
#endif

bool State::loadState(std::shared_ptr<DBService> &dbServer) {
  stateLock.lock();
  auto accounts = dbServer->readBatch(DBPrefix::nativeAccounts);
  if (accounts.empty()) {
    Address dev("0x21B782f9BF82418A42d034517CB6Bf00b4C17612", true);
    dbServer->put(dev.get(),Utils::uint256ToBytes(uint256_t("100000000000000000000")) + Utils::uint32ToBytes(0), DBPrefix::nativeAccounts);
    accounts = dbServer->readBatch(DBPrefix::nativeAccounts);
  }
  for (const auto account : accounts) {
    Address address(account.key, false);
    this->nativeAccount[address].balance = Utils::bytesToUint256(account.value.substr(0,32));
    this->nativeAccount[address].nonce = Utils::bytesToUint32(account.value.substr(32,4));
  }
  stateLock.unlock();
  return true;
}

bool State::saveState(std::shared_ptr<DBService> &dbServer) {
  stateLock.lock();
  WriteBatchRequest accountsBatch;
  for (auto &account : this->nativeAccount) {
    accountsBatch.puts.emplace_back(
      account.first.get(),
      Utils::uint256ToBytes(account.second.balance) + Utils::uint32ToBytes(account.second.nonce)
    );
  }
  dbServer->writeBatch(accountsBatch, DBPrefix::nativeAccounts);
  stateLock.unlock();
  return true;
}

bool State::validateTransactionForBlock(const Tx::Base& tx) const {
  // TODO: Handle error conditions to report at RPC level:
  // https://www.jsonrpc.org/specification#error_object
  // https://eips.ethereum.org/EIPS/eip-1474#error-codes
  // TODO: Handle transaction queue for multiple tx's from single user.

  if (!tx.verified()) {
    return false;
  }
  int err = 0;
  std::string errMsg;
  // Check if transaction is valid.
  // TX already exists...
  stateLock.lock_shared();
  if (this->mempool.count(tx.hash())) {
    stateLock.unlock_shared();
    return true;
  } else if (this->nativeAccount.count(tx.from()) > 0) {
    // Account already exists.
    // Use find() and count() instead of operator[] for const correctness and performance.
    if (this->nativeAccount.find(tx.from())->second.balance < tx.value()) {
      // Insufficient balance.
      stateLock.unlock_shared();
      return false;
    }
    if (this->nativeAccount.find(tx.from())->second.nonce != tx.nonce()) {
      // Invalid nonce.
      stateLock.unlock_shared();
      return false;
    }
  } else {
    // Account doesn't exists, meaning zero balance, meaning no balance to pay fees.
    stateLock.unlock_shared();
    return false;
  }
  stateLock.unlock_shared();

  return true;
}

std::pair<int, std::string> State::validateTransactionForRPC(const Tx::Base&& tx, const bool &broadcast) const {
  // TODO: Handle error conditions to report at RPC level:
  // https://www.jsonrpc.org/specification#error_object
  // https://eips.ethereum.org/EIPS/eip-1474#error-codes
  // TODO: Handle transaction queue for multiple tx's from single user.

  if (!tx.verified()) {
    return std::make_pair(-32003, "Transaction signature not verified when TX was constructed: " + Utils::bytesToHex(tx.rlpSerialize(true)));
  }
  int err = 0;
  std::string errMsg;
  // Check if transaction is valid.
  // TX already exists...
  stateLock.lock_shared();
  if (this->mempool.count(tx.hash())) {
    err = 0; errMsg = "NAN, Transaction already exists in mempool"; // Not really considered a failure

  } else if (this->nativeAccount.count(tx.from()) > 0) {
    // Account already exists.
    // Use find() and count() instead of operator[] for const correctness and performance.
    if (this->nativeAccount.find(tx.from())->second.balance < tx.value()) {
      // Insufficient balance.
      err = -32002;
      errMsg = "Insufficient balance- required: " +
      boost::lexical_cast<std::string>(tx.value()) + " available: " +
      boost::lexical_cast<std::string>(this->nativeAccount.find(tx.from())->second.balance);
    }
    if (this->nativeAccount.find(tx.from())->second.nonce != tx.nonce()) {
      // Invalid nonce.
      err = -32001;
      errMsg = "Invalid nonce";
    }
  } else {
    // Account doesn't exists, meaning zero balance, meaning no balance to pay fees.
    err = -32003; errMsg = "Insufficient balance - required: " +
    boost::lexical_cast<std::string>(tx.value()) + " available: 0";
  }
  stateLock.unlock_shared();

  if (err != 0) {
    errMsg.insert(0, "Transaction rejected: ");
    Utils::LogPrint(Log::subnet, "validateTransactionForRPC: ", errMsg);
  } else {
    stateLock.lock();
    std::string txHash = tx.hash();
    this->mempool[txHash] = std::move(tx);
    #if !IS_LOCAL_TESTS
      // Broadcast tx.
      if (broadcast) {
        // if I do
        // std::thread t(&grpcClient::relayTransaction, this->grpcClient, this->mempool[tx.hash()]);
        // t.detach()
        // It will only work half of the time.
        // TODO: figure out why and fix it
        this->grpcClient->relayTransaction(this->mempool[txHash]);
      } 
      //grpcClient->requestBlock();
    #endif
    stateLock.unlock();
  }
  return std::make_pair(err, errMsg);
}

bool State::processNewTransaction(const Tx::Base& tx) {
  bool isContractCall = false;
  Utils::LogPrint(Log::state, __func__, "tx.from(): " + tx.from().hex() + " tx.value(): " + boost::lexical_cast<std::string>(tx.value()));
  // TODO: Check Balance.
  // Remove transaction from mempool if found there.
  if (this->mempool.count(tx.hash()) != 0) this->mempool.erase(tx.hash());

  // Update Balances.s
  this->nativeAccount[tx.from()].balance -= tx.value();
  this->nativeAccount[tx.from()].balance -= uint256_t(tx.gasPrice() * tx.gas());
  this->nativeAccount[tx.to()].balance += tx.value();
  // Update nonce.
  this->nativeAccount[tx.from()].nonce++;


  // TODO: Handle contract calls.
  return true;
}


// Check block header, validation and transactions within.
// Invalid transactions (such as invalid signatures, account having min balance for min fees), if included in a block, the block will be rejected.
bool State::validateNewBlock(const Block &newBlock, const std::shared_ptr<const ChainHead>& chainHead) const {
  // Check block previous hash.
  auto bestBlock = chainHead->latest();
  if (bestBlock->getBlockHash() != newBlock.prevBlockHash()) {
    Utils::LogPrint(Log::state, __func__, "Block previous hash does not match.");
    Utils::LogPrint(Log::state, __func__, "newBlock previous hash: " + newBlock.prevBlockHash());
    Utils::LogPrint(Log::state, __func__, "bestBlock hash: " + bestBlock->getBlockHash());
    return false;
  }

  if (newBlock.nHeight() != (1 + bestBlock->nHeight())) {
    Utils::LogPrint(Log::state, __func__, "Block height does not match.");
    Utils::LogPrint(Log::state, __func__, "newBlock height: " + std::to_string(newBlock.nHeight()));
    Utils::LogPrint(Log::state, __func__, "bestBlock height: " + std::to_string(bestBlock->nHeight()));
    return false;
  }

  for (const auto &tx : newBlock.transactions()) {
    if (!this->validateTransactionForBlock(tx)) {
      Utils::LogPrint(Log::state, __func__, "Block rejected due to invalid transaction");
      return false;
    }
  }
  Utils::LogPrint(Log::state, __func__, "Block " + Utils::bytesToHex(newBlock.getBlockHash()) + ", height " + boost::lexical_cast<std::string>(newBlock.nHeight()) + " validated.");
  return true;
}


void State::processNewBlock(const std::shared_ptr<const Block>&& newBlock, const std::shared_ptr<ChainHead>& chainHead) {
  // Check block previous hash.
  this->stateLock.lock();
  Utils::LogPrint(Log::state, __func__, "Processing new block " + Utils::bytesToHex(newBlock->getBlockHash()) + ", height " + boost::lexical_cast<std::string>(newBlock->nHeight()));
  auto bestBlock = chainHead->latest();

  for (const auto &tx : newBlock->transactions()) {
    this->processNewTransaction(tx);
  }
  // Append block to chainHead.
  chainHead->push_back(std::move(newBlock));
  this->mempool.clear();
  this->stateLock.unlock();
}

const std::shared_ptr<const Block> State::createNewBlock(std::shared_ptr<ChainHead>& chainHead, std::shared_ptr<ChainTip> &chainTip) const {
  Utils::LogPrint(Log::state, __func__, "Creating new block.");
  auto bestBlockHash = chainTip->getPreference();
  std::shared_ptr<const Block> bestBlock;
  if (bestBlockHash.empty()) {
    // No prefered block found, use latest from chainHead.
    Utils::LogPrint(Log::state, __func__, "No prefered block found, using latest from chainHead.");
    bestBlock = chainHead->latest();
  } else {
    Utils::LogPrint(Log::state, __func__, std::string("Got preference: ") + Utils::bytesToHex(bestBlockHash));
    // TODO: Error handling if a block is not found.
    // Maybe refactor chainHead to return nullptrs if a block is not found?
    bestBlock = chainHead->getBlock(bestBlockHash);
    Utils::LogPrint(Log::state, __func__, "Got best block.");
  }
  auto newBestBlock = std::make_shared<Block>(
    Utils::bytesToUint256(bestBlock->getBlockHash()),
    std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::high_resolution_clock::now().time_since_epoch()
    ).count(),
    bestBlock->nHeight() + 1
  );

  stateLock.lock_shared();
  for (auto &tx : this->mempool) newBestBlock->appendTx(tx.second);
  newBestBlock->finalizeBlock();
  newBestBlock->indexTxs();
  stateLock.unlock_shared();
  Utils::LogPrint(Log::state, __func__, "New block created.");
  return newBestBlock;
}

uint256_t State::getNativeBalance(const Address& address) {
  uint256_t ret;
  this->stateLock.lock_shared();
  ret = this->nativeAccount[address].balance;
  this->stateLock.unlock_shared();
  return ret;
};

uint256_t State::getNativeNonce(const Address& address) {
  uint256_t ret;
  this->stateLock.lock_shared();
  ret = this->nativeAccount[address].nonce;
  this->stateLock.unlock_shared();
  return ret;
};

void State::addBalance(const Address &address) {
  this->stateLock.lock();
  this->nativeAccount[address].balance += 1000000000000000000;
  this->stateLock.unlock();
}
