/*
Copyright (c) [2023-2024] [Sparq Network]

This software is distributed under the MIT License.
See the LICENSE.txt file in the project root for more information.
*/

#ifndef BROADCASTER_H
#define BROADCASTER_H

#include <thread>

#include "rdpos.h"

#include "../utils/ecdsa.h"
#include "../utils/logger.h"
#include "../utils/strings.h"
#include "../utils/tx.h"

class Blockchain; // Forward declaration.

// TODO: tests for Consensus (if necessary)

/// Class responsible for processing blocks and transactions.
class Consensus {
  private:
    Blockchain& blockchain_; ///< Reference to the blockchain.
    std::future<void> loopFuture_;  ///< Future object holding the thread for the consensus loop.
    std::atomic<bool> canCreateBlock_ = false; ///< Flag for knowing if the worker is ready to create a block.
    std::atomic<bool> stop_ = false; ///< Flag for stopping the consensus processing.

    /**
     * Create and broadcast a Validator block (called by validatorLoop()).
     * If the node is a Validator and it has to create a new block,
     * this function will be called, the new block will be created based on the
     * current State and rdPoS objects, and then it will be broadcast.
     * @throw DynamicException if block is invalid.
     */
    void doValidatorBlock();

    /**
     * Wait for a new block (called by validatorLoop()).
     * If the node is a Validator, this function will be called to make the
     * node wait until it receives a new block.
     */
    void doValidatorTx() const;

    /**
     * Wait for transactions to be added to the mempool and create a block by rdPoS consensus. Called by workerLoop().
     * TODO: this function should call State or Blockchain to let them know that we are ready to create a block.
     */
    void doBlockCreation();

    /**
     * Create a transaction by rdPoS consensus and broadcast it to the network.
     * @param nHeight The block height for the transaction.
     * @param me The Validator that will create the transaction.
     */
    void doTxCreation(const uint64_t& nHeight, const Validator& me);

    /**
     * Sign a block using the Validator's private key.
     * @param block The block to sign.
     */
    void signBlock(Block& block);

  public:
    /**
     * Constructor.
     * @param blockchain Reference to the blockchain.
     */
    explicit Consensus(Blockchain& blockchain) : blockchain_(blockchain) {}

    /**
     * Entry function for the worker thread (runs the workerLoop() function).
     * @return `true` when done running.
     */
    bool workerLoop();
    void validatorLoop(); ///< Routine loop for when the node is a Validator.

    void start(); ///< Start the consensus loop. Should only be called after node is synced.
    void stop();  ///< Stop the consensus loop.
};

#endif  // BROADCASTER_H
