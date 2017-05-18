
#if !defined(FOR_EACH_ARITHM)
#  define FOR_EACH_ARITHM(arithm)
#endi

FOR_EACH_ARITHM(uint8_t )
FOR_EACH_ARITHM(uint16_t)
FOR_EACH_ARITHM(uint32_t)
FOR_EACH_ARITHM(uint64_t)

#undef FOR_EACH_ARITHM

#if !defined(FOR_EACH_SORT)
#  define FOR_EACH_SORT(sort)
#endif

FOR_EACH_SORT(std::sort)
FOR_EACH_SORT(radix_sort::sort)
FOR_EACH_SORT(radix_sort::concurrent_sort)

#undef FOR_EACH_SORT

#if !defined(FOR_EACH_ARITHM_SORT)
#  define FOR_EACH_ARITHM_SORT(arithm, sort)
#endif

FOR_EACH_ARITHM_SORT(uint8_t , std::sort                  )
FOR_EACH_ARITHM_SORT(uint16_t, std::sort                  )
FOR_EACH_ARITHM_SORT(uint32_t, std::sort                  )
FOR_EACH_ARITHM_SORT(uint64_t, std::sort                  )
FOR_EACH_ARITHM_SORT(uint8_t , radix_sort::sort           )
FOR_EACH_ARITHM_SORT(uint16_t, radix_sort::sort           )
FOR_EACH_ARITHM_SORT(uint32_t, radix_sort::sort           )
FOR_EACH_ARITHM_SORT(uint64_t, radix_sort::sort           )
FOR_EACH_ARITHM_SORT(uint8_t , radix_sort::concurrent_sort) 
FOR_EACH_ARITHM_SORT(uint16_t, radix_sort::concurrent_sort) 
FOR_EACH_ARITHM_SORT(uint32_t, radix_sort::concurrent_sort) 
FOR_EACH_ARITHM_SORT(uint64_t, radix_sort::concurrent_sort) 

#undef FOR_EACH_ARITHM_SORT

