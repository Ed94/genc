#pragma region Timing

#ifdef GEN_BENCHMARK
//! Return CPU timestamp.
GEN_DEF gen_u64 gen_read_cpu_time_stamp_counter( void );

//! Return relative time (in seconds) since the application start.
GEN_DEF gen_f64 gen_time_rel( void );

//! Return relative time since the application start.
GEN_DEF gen_u64 gen_time_rel_ms( void );
#endif

#pragma endregion Timing
