     1	from mfs import *
     2	
     3	class WriteTest(MfsTest):
     4	   name = "write"
     5	   description = "write then read one block"
     6	   timeout = 10
     7	
     8	   def run(self):
     9	      self.loadlib()
    10	      self.start_server()
    11	
    12	      self.mfs_init("localhost", self.port)
    13	      self.creat(0, MFS_REGULAR_FILE, "test")
    14	      inum = self.lookup(0, "test")
    15	
    16	      buf1 = gen_block(1)
    17	      self.write(inum, buf1, 0)
    18	
    19	      buf2 = BlockBuffer()
    20	      self.read(inum, buf2, 0)
    21	
    22	      if not bufs_equal(buf1, buf2):
    23	         raise Failure("Corrupt data returned by read")
    24	
    25	      self.shutdown()
    26	
    27	      self.server.wait()
    28	      self.done()
    29	
    30	class StatTest(MfsTest):
    31	   name = "stat"
    32	   description = "stat a regular file"
    33	   timeout = 10
    34	
    35	   def run(self):
    36	      self.loadlib()
    37	      self.start_server()
    38	
    39	      self.mfs_init("localhost", self.port)
    40	      self.creat(ROOT, MFS_REGULAR_FILE, "test")
    41	      inum = self.lookup(ROOT, "test")
    42	
    43	      st = self.stat(ROOT)
    44	      if st.type != MFS_DIRECTORY:
    45	         raise Failure("Stat gave wrong type")
    46	
    47	      st = self.stat(inum)
    48	      if st.size != 0:
    49	         raise Failure("Stat gave wrong size")
    50	      if st.type != MFS_REGULAR_FILE:
    51	         raise Failure("Stat gave wrong type")
    52	
    53	      buf1 = gen_block(1)
    54	      self.write(inum, buf1, 0)
    55	
    56	      st = self.stat(inum)
    57	      if st.size != MFS_BLOCK_SIZE:
    58	         raise Failure("Stat gave wrong size")
    59	      if st.type != MFS_REGULAR_FILE:
    60	         raise Failure("Stat gave wrong type")
    61	
    62	      self.shutdown()
    63	      self.server.wait()
    64	      self.done()
    65	
    66	
    67	class OverwriteTest(MfsTest):
    68	   name = "overwrite"
    69	   description = "overwrite a block"
    70	   timeout = 10
    71	
    72	   def run(self):
    73	      self.loadlib()
    74	      self.start_server()
    75	
    76	      self.mfs_init("localhost", self.port)
    77	      self.creat(0, MFS_REGULAR_FILE, "test")
    78	      inum = self.lookup(0, "test")
    79	
    80	      buf1 = gen_block(1)
    81	      self.write(inum, buf1, 0)
    82	      self.read_and_check(inum, 0, buf1)
    83	
    84	      buf2 = gen_block(2)
    85	      self.write(inum, buf2, 0)
    86	      self.read_and_check(inum, 0, buf2)
    87	
    88	      self.shutdown()
    89	
    90	      self.server.wait()
    91	      self.done()
    92	
    93	class MaxFileTest(MfsTest):
    94	   name = "maxfile"
    95	   description = "write largest possible file"
    96	   timeout = 10
    97	
    98	   def run(self):
    99	      self.loadlib()
   100	      self.start_server()
   101	
   102	      self.mfs_init("localhost", self.port)
   103	      self.creat(0, MFS_REGULAR_FILE, "test")
   104	      inum = self.lookup(0, "test")
   105	
   106	      buf = [gen_block(i) for i in range(MAX_FILE_BLOCKS)]
   107	
   108	      for i in range(MAX_FILE_BLOCKS):
   109	         self.write(inum, buf[i], i)
   110	
   111	      for i in range(MAX_FILE_BLOCKS):
   112	         self.read_and_check(inum, i, buf[i])
   113	
   114	      self.shutdown()
   115	
   116	      self.server.wait()
   117	      self.done()
   118	
   119	
   120	class MaxFile2Test(MfsTest):
   121	   name = "maxfile2"
   122	   description = "write more blocks than possible"
   123	   timeout = 10
   124	
   125	   def run(self):
   126	      self.loadlib()
   127	      self.start_server()
   128	      self.mfs_init("localhost", self.port)
   129	      self.creat(0, MFS_REGULAR_FILE, "test")
   130	      inum = self.lookup(0, "test")
   131	
   132	      buf = [gen_block(i) for i in range(MAX_FILE_BLOCKS + 1)]
   133	      for i in range(MAX_FILE_BLOCKS):
   134	         self.write(inum, buf[i], i)
   135	      i = MAX_FILE_BLOCKS
   136	      r = self.libmfs.MFS_Write(inum, byref(buf[i]), i)
   137	      if r != -1:
   138	         raise Failure("MFS_Write should fail on inalid block number")
   139	
   140	      for i in range(MAX_FILE_BLOCKS):
   141	         self.read_and_check(inum, i, buf[i])
   142	      i = MAX_FILE_BLOCKS
   143	      r = self.libmfs.MFS_Read(inum, byref(buf[i]), i)
   144	      if r != -1:
   145	         raise Failure("MFS_Read should fail on inalid block number")
   146	
   147	      self.shutdown()
   148	
   149	      self.server.wait()
   150	      self.done()
   151	
   152	class SparseTest(MfsTest):
   153	   name = "sparse"
   154	   description = "write first and last block"
   155	   timeout = 10
   156	
   157	   def run(self):
   158	      self.loadlib()
   159	      self.start_server()
   160	
   161	      self.mfs_init("localhost", self.port)
   162	      self.creat(0, MFS_REGULAR_FILE, "test")
   163	      inum = self.lookup(ROOT, "test")
   164	
   165	      buf = gen_block(1)
   166	      self.write(inum, buf, 0)
   167	      self.read_and_check(inum, 0, buf)
   168	
   169	      buf = gen_block(2)
   170	      self.write(inum, buf, MAX_FILE_BLOCKS - 1)
   171	      self.read_and_check(inum, MAX_FILE_BLOCKS - 1, buf)
   172	
   173	      buf = gen_block(3)
   174	      self.write(inum, buf, 0)
   175	      self.read_and_check(inum, 0, buf)
   176	      buf = gen_block(2)
   177	      self.read_and_check(inum, MAX_FILE_BLOCKS - 1, buf)
   178	
   179	      buf = gen_block(4)
   180	      self.write(inum, buf, MAX_FILE_BLOCKS - 1)
   181	      self.read_and_check(inum, MAX_FILE_BLOCKS - 1, buf)
   182	      buf = gen_block(3)
   183	      self.read_and_check(inum, 0, buf)
   184	
   185	      self.shutdown()
   186	      self.server.wait()
   187	      self.done()
   188	
   189	class Stat2Test(MfsTest):
   190	   name = "stat2"
   191	   description = "stat a sparse file"
   192	   timeout = 10
   193	
   194	   def run(self):
   195	      self.loadlib()
   196	      self.start_server()
   197	
   198	      self.mfs_init("localhost", self.port)
   199	      self.creat(0, MFS_REGULAR_FILE, "test")
   200	      inum = self.lookup(ROOT, "test")
   201	
   202	      buf = gen_block(1)
   203	      self.write(inum, buf, 0)
   204	      buf = gen_block(2)
   205	      self.write(inum, buf, MAX_FILE_BLOCKS - 1)
   206	      buf = gen_block(3)
   207	      self.write(inum, buf, 0)
   208	      buf = gen_block(4)
   209	      self.write(inum, buf, MAX_FILE_BLOCKS - 1)
   210	
   211	      st = self.stat(inum)
   212	      if st.type != MFS_REGULAR_FILE:
   213	         raise Failure("Stat gave wrong type")
   214	      if st.size != MAX_FILE_BLOCKS * MFS_BLOCK_SIZE:
   215	         raise Failure("Stat gave wrong size")
   216	
   217	      self.shutdown()
   218	      self.server.wait()
   219	      self.done()
   220	
   221	
   222	
   223	
   224	
   225	test_list = [WriteTest, StatTest, OverwriteTest, MaxFileTest, MaxFile2Test, SparseTest, Stat2Test]
