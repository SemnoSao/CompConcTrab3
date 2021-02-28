count_seq = 0
count_trinca = 0
ini_max_rep = 0
count_max_rep = 0
max_rep_num = -1

file = open("teste.bin", "rb")
size = int.from_bytes(file.read(8), "little")
numbers = []
pos = 0
rep_pos = 1
ant = -1
seq = 0
rep = 1
trin = 1
for i in range(size) :
    pos += 1
    
    num = int.from_bytes(file.read(4), "little")
    
    if num == seq:
        if seq == 5:
            seq = 0
            count_seq += 1
        else: 
            seq += 1
    else:
        seq = 0
    
    if num == ant:
        trin += 1
        rep += 1
        
        if trin == 3:
            trin = 1
            count_trinca += 1
    else:
        trin = 1
        if rep >= count_max_rep:
            count_max_rep = rep
            ini_max_rep = rep_pos
            max_rep_num = ant
        rep_pos = pos
        rep = 1
    
    ant = num


print(ini_max_rep, count_max_rep, max_rep_num)
print(count_trinca)
print(count_seq)