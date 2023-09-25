# CPU 快取模擬系統

將編譯完的執行檔命名為cache
### 執行程式
- `$ make` : 產生執行檔 cache
- `$ cache [cache size] [block size] [associativity] [replace policy] [test file name]` : 依參數規格進行模擬

### 變數規格
- cache size: 8, 16, …, 256 (KB)
- block size: 4, 8, 16, …, 128 (B)
- associativity: 1 (direct mapped), 2, 4, 8, f (fully associative)
- replace-policy: FIFO, LRU

### 程式輸出
- input file
- demand fetch
- cache hit
- cache miss
- miss rate
- read data
- write data
- byte from memory
- byte to memory 

### 測試檔案
Din檔內容格式
- Label
    - 0: data read
    - 1: data write
    - 2: instruction (read)
- Address: 由tag、index以及offset組成，並以16進位表示
