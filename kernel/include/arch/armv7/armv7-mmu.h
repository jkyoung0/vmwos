#define SCTLR_THUMB_EXCEPTION		(1<<30)
#define SCTLR_ACCESS_FLAG_ENABLE	(1<<29)
#define SCTLR_TEX_REMAP_ENABLE		(1<<28)
#define SCTLR_NONMASKABLE_FIQ		(1<<27)
#define SCTLR_EXCEPTION_ENDIANESS	(1<<25)
#define SCTLR_IRQ_VECTOR_ENABLE		(1<<24)
#define SCTLR_ALIGNMENT			(1<<22)
#define SCTLR_FAST_INTERRUPT_CONFIG	(1<<21)
#define SCTLR_UWXN			(1<<20)
#define SCTLR_WXN			(1<<19)
#define SCTLR_HARDWARE_ACCESS		(1<<17)
#define SCTLR_ROUND_ROBIT		(1<<14)
#define SCTLR_HIGH_VECTORS		(1<<13)
#define SCTLR_ICACHE_ENABLE		(1<<12)
#define SCTLR_BRANCH_PREDICTOR_ENABLE	(1<<11)
#define SCTLR_SWP_ENABLE		(1<<10)
#define SCTLR_CP15_BARRIER_ENABLE	(1<<5)
#define SCTLR_CACHE_ENABLE		(1<<2)
#define SCTLR_ALIGNMENT_CHECK		(1<<1)
#define SCTLR_MMU_ENABLE		(1<<0)

void enable_mmu(uint32_t mem_start, uint32_t mem_end, uint32_t kernel_end);
void enable_l1_dcache(void);
void disable_l1_dcache(void);
void enable_l1_icache(void);
void disable_l1_icache(void);
void enable_branch_predictor(void);
void disable_branch_predictor(void);

void flush_icache(void);
