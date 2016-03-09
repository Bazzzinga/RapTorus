## Here comes main graphical displays to output
import tkinter as tk
from math import floor
import QMM as qm

def SortV(vect):
    zvlist=[]
    for i in vect:
        z=(i[1][2]+i[2][2]+i[3][2])/3.0
        zv=(z,i)
        zvlist.append(zv)
    reZV =sorted(zvlist)
    outlist=[]
    for i in reZV:
        outlist.append(i[1])

    return outlist
        

class G3Dcnv:
    def __init__(self,cons,master):
        light=qm.normalize(cons.light) # vector of light direction
        width,height=cons.wsize
        
        self.canvas = tk.Canvas(master,
        width = cons.wsize[0],
        height = cons.wsize[1],
        bg = '#FFF')
        canvas=self.canvas
        print('canvas created')
        
        #cons.cur_position_xy=cons.position_xy# center of coordinates of the model

        def draw(canvas):
            rot_axis=cons.rot_axis
            angl=cons.angl
            cmass_x=cons.cmass_xyz[0]*cons.scale_factor
            cmass_y=cons.cmass_xyz[1]*cons.scale_factor
            cmass_z=cons.cmass_xyz[2]*cons.scale_factor
            vect=SortV(cons.vect)

            canvas.delete("all")
            cons.subvect=[]
            for vec in vect:

                #coordinates and normals
                csf=cons.scale_factor
                vec1=qm.rotAng((vec[1][0]-cmass_x/csf,vec[1][1]-cmass_y/csf,vec[1][2]-cmass_z/csf),rot_axis,angl)
                vec2=qm.rotAng((vec[2][0]-cmass_x/csf,vec[2][1]-cmass_y/csf,vec[2][2]-cmass_z/csf),rot_axis,angl)
                vec3=qm.rotAng((vec[3][0]-cmass_x/csf,vec[3][1]-cmass_y/csf,vec[3][2]-cmass_z/csf),rot_axis,angl)
                
                f_normal=(vec[0][0],vec[0][1],vec[0][2])
                f_normal=qm.normalize(f_normal)
                n_vec=qm.rotAng(f_normal,rot_axis,angl)
                
                z1=(vec1[0]*cons.scale_factor,vec1[1]*cons.scale_factor,vec1[2]*cons.scale_factor)
                z2=(vec2[0]*cons.scale_factor,vec2[1]*cons.scale_factor,vec2[2]*cons.scale_factor)
                z3=(vec3[0]*cons.scale_factor,vec3[1]*cons.scale_factor,vec3[2]*cons.scale_factor)

                bright_factor=sum((a*b) for a, b in zip(light, n_vec)) # scalar vector multiplication
                normal=abs(bright_factor)/1.5
                if bright_factor>0:
                    python_green = (int(abs(floor(205*normal+50))),int(abs(floor(205*normal+50))),int(abs(floor(205*normal+50))))
                else:
                    python_green =(0,0,0)
                tk_rgb = "#%02x%02x%02x" % python_green

                if n_vec[2]>0:
                    canvas.create_polygon((z1[0]+cons.cur_position_xy[0], z1[1]+cons.cur_position_xy[1],
                                            z2[0]+cons.cur_position_xy[0], z2[1]+cons.cur_position_xy[1],
                                            z3[0]+cons.cur_position_xy[0], z3[1]+cons.cur_position_xy[1]), fill=tk_rgb, outline='black')

                vec1=(vec1[0]+cmass_x/csf,vec1[1]+cmass_y/csf,vec1[2]+cmass_z/csf)
                vec2=(vec2[0]+cmass_x/csf,vec2[1]+cmass_y/csf,vec2[2]+cmass_z/csf)
                vec3=(vec3[0]+cmass_x/csf,vec3[1]+cmass_y/csf,vec3[2]+cmass_z/csf)
                cons.subvect.append([n_vec,vec1,vec2,vec3])

        def draw_loading(canvas):
            a=canvas.create_text(30,30,font=20,anchor='nw')
            canvas.insert(a,0,'RapTorus v1.16.0225 ; (tap and drag left mouse button)')

        def def_rot_center(event):
            x,y=event.x, event.y
            cons.start_rot=x,y
            draw(canvas)

        def new_base_vec(event):
            if cons.subvect!=[]:
                cons.vect=cons.subvect
                cons.angl=0
            draw(canvas)

        def do_rot_motion(event): # changing direction vector based on the position where cursor is
            x, y = event.x, event.y  
            rot_factor=cons.rot_factor # making direct vector go outta plane to have outta plane rotation
            bv=cons.base_vec
            new_vec=(x-cons.start_rot[0],y-cons.start_rot[1],rot_factor)
            new_vec=qm.normalize(new_vec)
            if bv!=new_vec:
                cons.rot_axis=qm.cross(bv,new_vec)
                cons.angl=qm.angle(bv,new_vec)
            draw(canvas)

        def do_scale(event):
            canvas.focus_set()
            if event.delta<0:
                cons.scale_factor=cons.scale_factor*10/11.0
            else:
                cons.scale_factor=cons.scale_factor*1.1
            draw(canvas)

        def start_position(event):
            x, y = event.x, event.y
            cons.cursor_start_xy=x,y
            draw(canvas)

        def new_position(event):
            cons.position_xy=cons.cur_position_xy
            draw(canvas)

        def do_translate_motion(event):
            x, y = event.x, event.y
            delta_pos=x-cons.cursor_start_xy[0],y-cons.cursor_start_xy[1]
            cons.cur_position_xy=cons.position_xy[0]+delta_pos[0],cons.position_xy[1]+delta_pos[1]
            draw(canvas)

        print('listen to bindings')
        canvas.bind("<B1-Motion>", do_rot_motion)
        canvas.bind("<ButtonPress-1>", def_rot_center)
        canvas.bind("<ButtonRelease-1>", new_base_vec)
        canvas.bind("<MouseWheel>",do_scale)
        canvas.bind("<B3-Motion>",do_translate_motion)
        canvas.bind("<ButtonPress-3>", start_position)
        canvas.bind("<ButtonRelease-3>", new_position)
        draw_loading(canvas)
        canvas.focus_set()
        #canvas.pack()



    
