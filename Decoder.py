def decoder(Temp_String):
    Temp_String = Temp_String.replace('],',']/')
    Servos,Pos,Time =Temp_String.split('(')[1].split(')')[0].split('/')
    Time = float(Time)
    Pos = eval(Pos.replace('-',''))
    Servos = Servos[:-1]+"']"
    Servos = Servos.replace('self.ids.',"'")
    Servos = Servos.replace(',',"',")
    Servos = eval(Servos)
    return Servos,Pos,Time
