// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "primitives/block.h"

#include "hash.h"
#include "tinyformat.h"
#include "utilstrencodings.h"
#include "crypto/common.h"
#include "chainparams.h"

unsigned char GetNfactor(int64_t nTimestamp) {
    int l = 0;

    if (nTimestamp <= Params().GetConsensus().nChainStartTime)
        return Params().GetConsensus().nMinNFactor;

    int64_t s = nTimestamp - Params().GetConsensus().nChainStartTime;
    while ((s >> 1) > 3) {
      l += 1;
      s >>= 1;
    }

    s &= 3;

    int n = (l * 158 + s * 28 - 2670) / 100;

    if (n < 0) n = 0;

    if (n > 255)
        printf( "GetNfactor(%lld) - something wrong(n == %d)\n", nTimestamp, n );

    unsigned char N = (unsigned char) n;
    //printf("GetNfactor: %d -> %d %d : %d / %d\n", nTimestamp - nChainStartTime, l, s, n, min(max(N, minNfactor), maxNfactor));

    return std::min(std::max(N, Params().GetConsensus().nMinNFactor), Params().GetConsensus().nMaxNFactor);
}


uint256 CBlockHeader::GetHash() const
{
    return SerializeHash(*this);
}

// todo mainnet may fail with this version of function
uint256 CBlockHeader::GetPoWHash(int nHeight) const
{
    uint256 thash;
    uint64_t T = 1, R = 4, C = 4;

    /*switch (true) {
        case
    }*/

         if (nHeight >   188) { T =    1; R = 1024; C =  512; }
    else if (nHeight >   187) { T =    1; R =   16; C =  256; }
    else if (nHeight >   186) { T =    1; R =  256; C =   16; }
    else if (nHeight >   185) { T =    1; R =   32; C =   32; }
    else if (nHeight >   172) { T =    1; R =   32; C =    4; }
    else if (nHeight >   136) { T =    4; R =    4; C =    4; }

    lyra2re2_hash_n(BEGIN(nVersion), BEGIN(thash), T, R, C);

    return thash;
}

/*
uint256 CBlockHeader::GetPoWHash(int nHeight) const
{
   uint256 thash;
   if((Params().NetworkIDString() == CBaseChainParams::TESTNET && nHeight >= 0) || nHeight >= 347000) // New Lyra2re2 Testnet
   {
   	lyra2re2_hash(BEGIN(nVersion), BEGIN(thash));
   }
   else if(nHeight >= 208301)
   {
   	lyra2re_hash(BEGIN(nVersion), BEGIN(thash));
   }
   else
   {
   	scrypt_N_1_1_256(BEGIN(nVersion), BEGIN(thash), GetNfactor(nTime));
   }
   return thash;
}
*/

std::string CBlock::ToString() const
{
    std::stringstream s;
    s << strprintf("CBlock(hash=%s, ver=0x%08x, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=%08x, nNonce=%u, vtx=%u)\n",
        GetHash().ToString(),
        nVersion,
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        nTime, nBits, nNonce,
        vtx.size());
    for (unsigned int i = 0; i < vtx.size(); i++)
    {
        s << "  " << vtx[i]->ToString() << "\n";
    }
    return s.str();
}

int64_t GetBlockWeight(const CBlock& block)
{
    // This implements the weight = (stripped_size * 4) + witness_size formula,
    // using only serialization with and without witness data. As witness_size
    // is equal to total_size - stripped_size, this formula is identical to:
    // weight = (stripped_size * 3) + total_size.
    return ::GetSerializeSize(block, SER_NETWORK, PROTOCOL_VERSION | SERIALIZE_TRANSACTION_NO_WITNESS) * (WITNESS_SCALE_FACTOR - 1) + ::GetSerializeSize(block, SER_NETWORK, PROTOCOL_VERSION);
}
