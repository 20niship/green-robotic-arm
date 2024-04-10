import csv
from lxml import etree
import numpy as np
import logging
from colorlog import ColoredFormatter


def setup_logger():
    formatter = ColoredFormatter(
        "%(log_color)s%(levelname)-8s%(reset)s %(blue)s%(message)s",
        datefmt=None,
        reset=True,
        log_colors={
            "DEBUG": "cyan",
            "INFO": "green",
            "WARNING": "yellow",
            "ERROR": "red",
            "CRITICAL": "red",
        },
    )
    logger = logging.getLogger("example")
    handler = logging.StreamHandler()
    handler.setFormatter(formatter)
    logger.addHandler(handler)
    logger.setLevel(logging.DEBUG)
    return logger 
logger = setup_logger()
import os

def readcsv():
    with open('output.csv', 'r') as csvfile:
        reader = csv.reader(csvfile)
        rows = [row for row in reader]
        return rows
posData = readcsv()

def defineLinkImpl_(robot: etree.Element, name: str):
    child_ = get_child_names(name)
    if len(child_) == 0:
        logger.warning("No child found for joint " + name)
        return

    link = etree.Element("link", name=name)
    for m in child_:
        vis = etree.Element("visual")
        geom = etree.Element("geometry")
        mm = etree.Element("mesh", size="0.1 0.1 0.1", filename="meshes/" + m + ".obj")
        geom.append(mm)
        vis.append(geom)
        link.append(vis)
    robot.append(link)


def defineLinks(robot: etree.Element):
    for j in jointInfos:
        name = j["name"]
        defineLinkImpl_(robot, name)
    defineLinkImpl_(robot, "base")

jointInfos = [
    {
        "name": "base-yaw-cylinder",
        "parent": "base",
        "axis": "0 0 1",
        "lower": "-3.14",
        "upper": "3.14",
    },
    {
        "name": "link1-1-1",
        "parent": "base-yaw-cylinder",
        "lower": "0.00",
        "upper": "1.57"
    },
    {
        "name": "link2-1-1",
        "parent": "link1-1-1",
        "axis":"0 1 0",
        "lower": "-0.2",
        "upper": "1.57",
    },
    {
        "name": "link3-1-1",
        "parent": "link2-1-1",
        "lower": "-6.31",
        "upper": "0.50",
    },
    {
        "name": "link2-1",
        "parent": "link3-1-1",
        "lower": "-7.2",
        "upper": "1.00",
    },
]

def fmt_i(i: int):
    return "." + str(i).zfill(3)

def get_child_names(jointname: str):
    return [j[0] for j in posData if j[7] == jointname]

def getJointOrigin(jointname: str):
    for p in posData:
        if p[0] == jointname:
            print("FOUND   ", jointname, " in csv file", p[1], p[2], p[3])
            x = [(float(x)) for x in p[1:5]]
            # return np.array(x[0:3])
            return np.array([-x[1], x[0], x[2]])
    logger.error(jointname+ " not found in csv file")
    return np.array([0, 0, 0])


def defineJoints(robot: etree.Element):
    for j in jointInfos:
        typename = j["type"] if "type" in j else "revolute"
        joint = etree.Element("joint", name=j["name"], type=typename)
        parent = etree.Element("parent", link=j["parent"])
        child_ = j["name"]
        child = etree.Element("child", link=child_)
        joint.append(child)

        joint.append(parent)
        pos = getJointOrigin(j["name"]) # - getJointOrigin(j["parent"])

        xyz = str(pos[0]) + " " + str(pos[1]) + " " + str(pos[2])
        origin = etree.Element("origin", xyz=xyz, rpy="0 0 0")
        joint.append(origin)
        if "axis" in j:
            axis = etree.Element("axis", xyz=j["axis"])
            joint.append(axis)
        else:
            axis = etree.Element("axis", xyz="0 0 1")
        lower = j["lower"] if "lower" in j else "-3"
        upper = j["upper"] if "upper" in j else "3"
        limit = etree.Element("limit", lower=lower, upper=upper, effort="0", velocity="1.0")
        joint.append(axis)
        joint.append(limit)
        robot.append(joint)


def run():
    root = etree.Element("robot", name="my_robot")
    defineLinks(root)
    defineJoints(root)

    tree = etree.ElementTree(root)
    tree.write("blade_builder.urdf", pretty_print=True,
               xml_declaration=True, encoding="utf-8")

if __name__ == "__main__":
    run()
