def eSTL(path):
    f=open(path,'r')
    h=f.readlines()
    allal=[]
    for line in h:
        line=line.split(' ')
        if 'normal' in line:
            #print ('normal: ',line[-3],line[-2],line[-1][:-1])
            allal.append((line[-3],line[-2],line[-1][:-1]))
        if 'vertex' in line:
            #print ('vertex: ',line[-3],line[-2],line[-1][:-1])
            allal.append((line[-3],line[-2],line[-1][:-1]))
    f.close()

    k=0
    tri=[]
    point=[]
    for i in allal:
        point.append(i)
        k=k+1
        if k ==4:
            k=0
            tri.append(point)
            point=[]

    vect=[]
    for tr in tri:
        t=[]
        for p in tr:
            f=[]
            for z in p:
                f.append(float(z))
            t.append(f)
        vect.append(t)

    ######cmass of vertices <---- (change to real cmass)
    xvert=[]
    yvert=[]
    zvert=[]
    for v in vect:
        xvert.append((v[1][0]+v[2][0]+v[3][0])/3.0)
        yvert.append((v[1][1]+v[2][1]+v[3][1])/3.0)
        zvert.append((v[1][2]+v[2][2]+v[3][2])/3.0)
    x=sum(xvert)/len(xvert)
    y=sum(yvert)/len(yvert)
    z=sum(zvert)/len(zvert)
    cmass=(x,y,z)
    return vect,cmass
