def create_files(count = 9):
        for i in range(1,count+1):
                f = open(str(i)+".rc","w")
                f.close()
create_files()
