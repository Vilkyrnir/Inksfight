#!/usr/bin/env python

import sys
import xml.etree.ElementTree as ET
import re
import math

iFile = None
oFile = None

iniContent = ""
levelContent = ""
spawnersList = ""
childrenList = ""

for i, arg in enumerate(sys.argv):
	if (arg == "-i" and i+1 <= len(sys.argv)-1):
		iFile = sys.argv[i+1]
	elif (arg == "-o" and i+1 <= len(sys.argv)-1):
		oFile = sys.argv[i+1]

if (iFile == None):
	sys.exit("Usage: python3 TiledParser.py -i FILE [-o FILE]")

#Default output filename
if (oFile == None):
	oFile = "output.ini"

try:
	tree = ET.parse(iFile)
except:
	exit("Error: please provide a valid Tiled tmx file.")

tmxRoot = tree.getroot()

tilesetE = tmxRoot.find("tileset")

if (tilesetE == None):
	sys.exit("Error: no tileset element found.")

tsxPath = re.sub(r'(.*\/)(.*\.tmx)', r'\1', iFile) + tilesetE.get('source')
filenameWE = re.sub(r'(.*\/)(.*)\.tmx', r'\2', iFile)
levelContent = "[" + filenameWE[0].upper() + filenameWE[1:] + "]\r\n"
levelContent += "Position = (0,0,0)\r\n"
levelContent += "TrackList = StoreID\r\n"
try:
	tree = ET.parse(tsxPath)
except:
	exit("Error: could not load tsx file.")

tsxRoot = tree.getroot()

tsxImageWidth = int(tsxRoot.find("image").get('width'))
tsxImageHeight = int(tsxRoot.find("image").get('height'))
tileWidth = int(tsxRoot.get("tilewidth"))
tileHeight = int(tsxRoot.get("tileheight"))

staticBody = 0
staticBodyContent = ""
chainShapes = {}
chainShapeN = 1
usedTiles = []

tsxTilesX = tsxImageWidth/tileWidth
tsxTilesY = tsxImageHeight/tileHeight

for objectgroup in tmxRoot.find("objectgroup"):
	if (objectgroup.get("type") == "Spawn"):
		x = math.floor(float(objectgroup.get("x")))
		y = math.floor(float(objectgroup.get("y")))

		if ("width" in objectgroup.attrib):
			x = x+math.floor(float(objectgroup.get("width"))/2)
		if ("height" in objectgroup.attrib):
			y = y+math.floor(float(objectgroup.get("height"))/2)

		spawnerName = objectgroup.get("name")
		iniContent += f'[{spawnerName}]\r\nPosition = ({x}, {y}, 0)\r\n'
		spawnersList += f" {spawnerName} #"

		props = objectgroup.find("properties")
		if (props != None):
			for prop in props:
				iniContent += f'{prop.get("name")} = {prop.get("value")}\r\n'

		iniContent += "\r\n"

	elif (objectgroup.get("type") == "Enemy"):
		enemyName = objectgroup.get("name")
		enemyX = objectgroup.get("x")
		enemyY = objectgroup.get("y")
		iniContent += f"[{enemyName}Object]\r\n"
		iniContent += f"Position = ({enemyX}, {enemyY}, 0)\r\n"
		iniContent += f"Body = {enemyName}Body\r\n\r\n"

		iniContent += f"[{enemyName}Body]\r\n"
		iniContent += "Dynamic = false\r\n"
		iniContent += f"PartList = {enemyName}BodyPart\r\n\r\n"

		iniContent += f"[{enemyName}BodyPart]\r\n"
		iniContent += "Type = box\r\nSolid = false\r\n"
		props = objectgroup.find("properties")
		if (props != None):
			for prop in props:
				iniContent += f'{prop.get("name")} = {prop.get("value")}\r\n'

		iniContent += "\r\n"
	elif (objectgroup.get("type") == "Dynamic"):
		type = objectgroup.get("type")
		id = objectgroup.get("id")
		namePrefix = f'{objectgroup.get("type")}{objectgroup.get("id")}'
		gid = objectgroup.get("gid")
		#xPos = objectgroup.get("x")
		#yPos = objectgroup.get("y")
		iniContent += f'[{namePrefix}Object]\r\n'
		childrenList += f" {namePrefix}Object #"
		#iniContent += f"Position = ({xPos}, {yPos}, 0)\r\n"
		gid = int(objectgroup.get("gid"))-1
		iniContent += f'Graphic = Tiles{gid}\r\n'
		usedTiles.append(f'Tiles{objectgroup.get("gid")}')
		dChainShapeContent = ""
		trackContent = ""
		for prop in objectgroup.find("properties"):
			if (prop.get("name") == "DChainShape"):
				DChainShapeId = prop.get("value")
				iniContent += f"Body = DChainShape{DChainShapeId}\r\n"


				dChainShapeContent += f"\r\n[DChainShape{DChainShapeId}]\r\n"
				dChainShapeContent += "Dynamic = false\r\n"
				dChainShapeContent += "PartList = @\r\n"
				dChainShapeContent += "Type = box\r\n"
				dChainShapeContent += "Solid = true\r\n"
				dChainShapeContent += "CheckMask = player\r\n"
				dChainShapeContent += "SelfFlags = platforms\r\n"
				dChainShapeContent += "Friction = 1\r\n"
			elif (prop.get("name") == "PathStart"):
				startId = prop.get("value")
				endId = ''
				for prop in objectgroup.find("properties"):
					if (prop.get("name") == "PathEnd"):
						endId = prop.get("value")
				if (endId == ''):
					system.exit("Error: PathStart property found without PathEnd.")
				tempTree = ET.parse(iFile)
				rootNode = tempTree.getroot()
				iniContent += f"TrackList = {namePrefix}CheckTrack\r\n"
				pathStartNode = rootNode.find(f'.//*[@id="{startId}"]')
				pathStartX = pathStartNode.get("x")
				pathStartY = pathStartNode.get("y")
				# Here we set our object position with path start coordinates
				iniContent += f"Position = ({pathStartX}, {pathStartY}, 0)\r\n"
				pathEndNode = rootNode.find(f'.//*[@id="{endId}"]')
				pathEndX = pathEndNode.get("x")
				pathEndY = pathEndNode.get("y")
				trackContent += f"\r\n[{namePrefix}CheckTrack]\r\n"
				trackContent += f"0  = Object_MoveBetween ^ ({pathStartX}, {pathStartY}, 0) ({pathEndX}, {pathEndY}, 0) 1\r\n"
				trackContent += "Loop = true\r\n\r\n"

		iniContent += dChainShapeContent
		iniContent += trackContent
	elif (objectgroup.get("type") == "ChainShape"):
		if (staticBody == 0):
			# We make a global object that will handle all static physic objects
			# that need collision
			staticBodyContent += "[StaticBodyObject]\r\n"
			staticBodyContent += "Position = (0, 0, 0)\r\n"
			staticBodyContent += "Body = StaticBody\r\n\r\n"

			staticBodyContent += "[StaticBody]\r\n"
			staticBodyContent += "Dynamic = false\r\n"
			staticBodyContent += "PartList = "
			staticBody += 1
			
			childrenList += " StaticBodyObject #"

		shapeName = f"ChainShape{chainShapeN}"
		chainShapes[shapeName] = f"\r\n\r\n[{shapeName}]\r\nType = chain\r\nFriction = 1\r\nLoop = true\r\nSolid = true\r\n"

		for prop in objectgroup.find("properties"):
			chainShapes[shapeName] += f'{prop.get("name")} = {prop.get("value")}\r\n'

		chainShapes[shapeName] += "VertexList = "

		baseX = math.floor(float(objectgroup.get("x")))
		baseY = math.floor(float(objectgroup.get("y")))

		points = objectgroup.find("polygon").get("points")
		points = points.split(" ")

		for point in points:
			point = point.split(",")
			x = baseX + math.floor(float(point[0]))
			y = baseY + math.floor(float(point[1]))

			chainShapes[shapeName] += f"({x}, {y}, 0) # "

		chainShapes[shapeName] = chainShapes[shapeName].rstrip(" # ")
		chainShapeN += 1;

if (len(spawnersList) > 0):
	levelContent += f"Spawners = {spawnersList[:-1]}\r\n"
if (len(childrenList) > 0):
	levelContent += f"ChildList = {childrenList[:-1]}\r\n"
if (len(chainShapes) > 0):
	iniContent += staticBodyContent
	iniContent += " # ".join(chainShapes.keys())
	iniContent += f'{"".join(chainShapes.values())}\r\n\r\n'

iniContent += "[TilesGraphic]\r\n";
iniContent += "Texture = " + tsxRoot.find("image").get("source") + "\r\n"
iniContent += "Pivot = top left\r\n"
iniContent += f"TextureSize = ({tileWidth}, {tileHeight}, 0)\r\n"
x = 0
y = 0
tilesArray = {}
tilesContent = {}

for tile in tsxRoot.findall("tile"):
	tileId = tile.get("id")
	mul = int(tileId)
	x = int(mul%tsxTilesX*tileWidth)
	name = ""
	y = math.floor(mul/tsxTilesX)*tileHeight

	for prop in tile.find("properties"):
		if (prop.get("name") == "name"):
			name = prop.get("value")

	tilesContent["Tiles" + tileId] = f"\r\n[Tiles{tileId}@TilesGraphic]\r\nTextureOrigin = ({x}, {y}, 0)\r\n"

	tilesArray[tileId] = "Tiles" + tileId

layersContent = ""

for layer in tmxRoot.findall("layer"):
	tilesCountX = int(layer.get("width"))
	tilesCountY = int(layer.get("height"))

	layersContent += "\r\n[" + layer.get("name") + "]"
	rowCount = 1
	colCount  = 0
	rowString = ""
	mDataArray = layer.find("data").text.replace("\n", "").split(",")
	layersContent += f"\r\nSize = ({tilesCountX}, {tilesCountY}, 0)"

	for key in mDataArray:
		if (colCount == 0):
			rowString += f"\r\nRow{rowCount} = "

		key = "0" if int(key) == 0 else str(int(key)-1)

		if (key in tilesArray):
			rowString += tilesArray[key] + " # "
			usedTiles.append(tilesArray[key])
		else:
			rowString += "NONE # "

		colCount += 1

		if (colCount == tilesCountX):
			rowString = rowString.rstrip(" # ") + ("\r\n" if rowCount == tilesCountY else "")
			colCount = 0
			rowCount += 1

	layersContent += rowString

usedTiles = list(dict.fromkeys(usedTiles))

for tile in tilesArray.values():
	if (tile not in usedTiles):
		tilesContent.pop(tile)

iniContent += "".join(tilesContent.values()) + layersContent

iniContent = levelContent + "\r\n" + iniContent

f = open(oFile,"w")
f.write(iniContent)
f.close()
