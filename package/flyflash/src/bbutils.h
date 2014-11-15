int recursive_action(const char *fileName,
		unsigned flags,
		int (*fileAction)(const char *fileName, struct stat *statbuf, void* userData, int depth),
		int (*dirAction)(const char *fileName, struct stat *statbuf, void* userData, int depth),
		void* userData,
		unsigned depth);

//int replace_equal(const char* src_path, const char* dst_path);
/* Some useful definitions */
#undef FALSE
#define FALSE   ((int) 0)
#undef TRUE
#define TRUE    ((int) 1)
#undef SKIP
#define SKIP	((int) 2)

#define LONE_DASH(s)     ((s)[0] == '-' && !(s)[1])
#define NOT_LONE_DASH(s) ((s)[0] != '-' || (s)[1])
#define LONE_CHAR(s,c)     ((s)[0] == (c) && !(s)[1])
#define NOT_LONE_CHAR(s,c) ((s)[0] != (c) || (s)[1])
#define DOT_OR_DOTDOT(s) ((s)[0] == '.' && (!(s)[1] || ((s)[1] == '.' && !(s)[2])))


enum {
	ACTION_RECURSE        = (1 << 0),
	ACTION_FOLLOWLINKS    = (1 << 1),
	ACTION_FOLLOWLINKS_L0 = (1 << 2),
	ACTION_DEPTHFIRST     = (1 << 3),
	/*ACTION_REVERSE      = (1 << 4), - unused */
	ACTION_QUIET          = (1 << 5),
};
