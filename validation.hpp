#pragma once

// Index validation
bool validate_index_ranges(bool verbose = true);
// Volume-based checks
void count_inverted_and_degenerate();
// Duplicate tet detection
void detect_duplicate_tets();
//Face manifold check:
void face_manifold_check();
