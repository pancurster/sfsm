#!/usr/bin/python
import sys
from graphviz import Digraph

class Gengraph(object):
    def __init__(self):
        pass

    def getTrName(self, line):
        line = line.strip(' ')
        name = line.split(' ')
        name.remove('tr_t')
        for n in name:
            if len(n) > 0:
                x = n.find('[')
                return n[:x];

    def createEdge(self, trdefinition):
        # prevent from adding multiple same named nodes 
        nodeSet = set()
        for line in trdefinition:
            # now we have list of names: state1, state2, event, event handler
            tok = line.split(',')
            # no enought tokens (empty line?)
            if len(tok) < 3:
                continue

            for i in range(len(tok)):
                tok[i] = tok[i].strip(' ')

            # adding nodes to graph
            if (not tok[0] in nodeSet):
                nodeSet.add(tok[0])
                self.dot.node(tok[0], tok[0], shape="box")
            if ((not tok[1] in nodeSet) and (tok[1] != "FSM_NO_STATE")):
                nodeSet.add(tok[1])
                self.dot.node(tok[1], tok[1], shape="box")
            
            if (tok[2] == "FSM_DEF_TR"):
                self.dot.edge(tok[0], tok[1], label="["+tok[3]+"]  ")
            elif (tok[1] == "FSM_NO_STATE"):
                self.dot.edge(tok[0], tok[0], label=tok[2]+"["+tok[3]+"]  ")
            else:
                self.dot.edge(tok[0], tok[1], label=tok[2]+"["+tok[3]+"]  ")
    
    def gen(self, filename):

        f = open(filename)
        src = f.readlines()

        found = 0
        trdefinition = []
        for line in src:
            if (line.find('tr_t') >= 0):
                found = 1
                self.dot = Digraph(comment=self.getTrName(line))
                print "Name: ", self.getTrName(line)
                continue

            if (found and line.find('};') >= 0):
                break

            if (found):
                line = line.strip(' ')
                if (line.startswith('/*') or line.endswith('*/') or line.startswith('*')):
                    continue
                line = line.replace('{', '')
                line = line.replace('}', '')
                trdefinition.append(line)
    
        # parsing each line and creating dot graph
        self.createEdge(trdefinition)

        print self.dot.source
        self.dot.render("out.gv")

        print 'finded definition:'
        for line in trdefinition:
            print line

        print '...generating fsm graph'

    def usage(self, argv):
        print 'Usage:'
        print argv[0], "src tr_table_name"

if __name__ == '__main__':
    g = Gengraph()

    if (len(sys.argv) < 2):
        g.usage(sys.argv)
        exit(1)

    g.gen(sys.argv[1])
