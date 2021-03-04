uint32_t hash_packet(struct mesh_msg *msg);

void init_hash();

void hash_add(uint32_t hash);

bool hash_contains(uint32_t hash_val);