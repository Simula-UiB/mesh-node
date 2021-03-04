/**
 * @brief Get the hash for a packet.
 * 
 * @details Only the fields that don't change between hops is included in the
 *          hash. This allows the same original packet to have the same hash
 *          even when the source MAC and TTl has changed.
 */
uint32_t hash_packet(struct mesh_msg *msg);

/**
 * @brief Initialize hash
 */
void init_hash();

/**
 * @brief Add a hash to the hashset, and remove old hashes if it is full.
 */
void hash_add(uint32_t hash);

/**
 * @brief Check if a given hash is already in the set.
 */
bool hash_contains(uint32_t hash_val);