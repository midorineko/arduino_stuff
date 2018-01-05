from rgb_cie import Converter
from PIL import ImageGrab
import httplib2
import time
from qhue import Bridge
import urllib.request

setting_device = 'elbLovRPUcHaqss904iEJMH9LZrRwsvFeOKSfvOP'
setting_bridge = '192.168.0.103'
setting_light_count = 4
b = Bridge(setting_bridge, setting_device)

globvar = 0

def set_globvar_one(x):
    global globvar    # Needed to modify global copy of globvar
    globvar = x
set_globvar_one(0)
def set_globvar_two(x):
    global globvar_two    # Needed to modify global copy of globvar
    globvar_two = x
set_globvar_two(0)
def mainLoop():

    while True:
        start()

def start():
    try:
        x, y, t, e = getPixels()
    except:
        mainLoop()
    # changeLight(x,y)
    time.sleep(.05)

def changeLight(x, y):
    x = round(x, 2)
    y = round(y, 2)
    if globvar - .01 < x < globvar + .01:
        x = globvar
    set_globvar_one(x)
    if globvar_two - .01 < y < globvar_two + .01:
        y = globvar_two
    set_globvar_two(y)

    # for t in range(1,setting_light_count+1):
    # b.lights(1, 'state', bri=255, on=True, xy=[x, y]);
    # b.lights(2, 'state', bri=255, on=True, xy=[x, y]);
    # b.lights(3, 'state', bri=255, on=True, xy=[x, y]);
    # b.lights(4, 'state', bri=255, on=True, xy=[x, y]);

def getPixels():
        #grab screenshot and get the size
        image = ImageGrab.grab()
        im = image.load()
        maxX, maxY = image.size
        step = 100
        #loop through pixels for rgb data
        data = []
        for y in range(0, maxY, step):
            for x in range(0, maxX, step):
                pixel = im[x,y]
                data.append(pixel)

        #loop and check for white/black to exclude from averaging
        r = 0
        g = 0
        b = 0
        threshMin = 30
        threshMax = 255
        counter = 0
        counter_2 = 0
        r_0 = 0
        g_0 = 0
        b_0 = 0
        r_1 = 0
        g_1 = 0
        b_1 = 0
        r_2 = 0
        g_2 = 0
        b_2 = 0
        r_3 = 0
        g_3 = 0
        b_3 = 0
        chunks = len(data)/4 
        for z in range(len(data)):
            rP, gP, bP = data[z]
            if rP > threshMax and gP > threshMax and bP > threshMax or rP < threshMin and gP < threshMin and bP < threshMin:
                pass
            else:
                r+= rP
                g+= gP
                b+= bP
                if counter_2 > chunks * 3:
                    r_0 += rP
                    g_0 += gP
                    b_0 += bP
                elif counter_2 > chunks * 2:
                    r_1 += rP
                    g_1 += gP
                    b_1 += bP
                elif counter_2 > chunks:
                    r_2 += rP
                    g_2 += gP
                    b_2 += bP
                elif counter_2 > 0:
                    r_3 += rP
                    g_3 += gP
                    b_3 += bP
                counter+= 1
                counter_2+= 1
        if counter > 0:        
            rAvg = r/counter
            gAvg = g/counter
            bAvg = b/counter
            rAvg_0 = r_0/(counter/4)
            gAvg_0 = g_0/(counter/4)
            bAvg_0 = b_0/(counter/4)
            rAvg_1 = r_1/(counter/4)
            gAvg_1 = g_1/(counter/4)
            bAvg_1 = b_1/(counter/4)
            rAvg_2 = r_2/(counter/4)
            gAvg_2 = g_2/(counter/4)
            bAvg_2 = b_2/(counter/4)
            rAvg_3 = r_3/(counter/4)
            gAvg_3 = g_3/(counter/4)
            bAvg_3 = b_3/(counter/4)


            converter = Converter()
            request_string = "http://192.168.0.110/ambient_loop?r="+str(rAvg)+'&g='+str(gAvg)+'&b='+str(bAvg)
            request_string_0 = "http://192.168.0.110/ambient_loop?r="+str(rAvg_0)+'&g='+str(gAvg_0)+'&b='+str(bAvg_0)
            request_string_1 = "http://192.168.0.110/ambient_loop?r="+str(rAvg_1)+'&g='+str(gAvg_1)+'&b='+str(bAvg_1)
            request_string_2 = "http://192.168.0.110/ambient_loop?r="+str(rAvg_2)+'&g='+str(gAvg_2)+'&b='+str(bAvg_2)
            request_string_3 = "http://192.168.0.110/ambient_loop?r="+str(rAvg_3)+'&g='+str(gAvg_3)+'&b='+str(bAvg_3)
            print(request_string)
            print(request_string_0)
            print(request_string_1)
            print(request_string_2)
            print(request_string_3)

            urllib.request.urlopen(request_string).read()

            # hueColor = converter.rgbToCIE1931(rAvg, gAvg, bAvg)
            return 'meow'
        else:
            print('problem')
            return (0,0)

mainLoop()
