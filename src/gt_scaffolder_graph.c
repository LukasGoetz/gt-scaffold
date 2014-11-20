/* Gt-Scaffolder auf Basis des SGA-Algorithmus
   written by Dorle Osterode, Lukas Goetz, Stefan Dang
   <stefan.dang@studium.uni-hamburg.de>
   Encoding: UTF-8
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "gt_scaffolder_graph.h"
#include "core/queue_api.h"
#include "core/types_api.h"

/* Datenstruktur Scaffold-Graph */

#define ANTISENSE 1
#define SENSE 0
#define REVERSE 1
#define SAME 0

typedef enum { GS_UNVISITED, GS_REPEAT, GS_POLYMORPHIC, GS_INCONSISTENT,
               GS_VISITED, GS_PROCESSED } GraphState;

/* Knoten */
struct GtScaffoldGraphVertex{
  /* eindeutige ID fuer den Knoten */
  GtUword id;
  /* Laenge der Sequenz, die der Contig darstellt */
  GtUword seqlen;
  /* Wert der A-Statistik, um Contigs als REPEAT oder UNIQUE
     klassifizieren zu koennen;
     in Genom-Tools vom Typ float */
  float astat;
  /* abgeschaetzte Anzahl an Vorkommen des Contigs im Genom */
  float copynum;  
  GtUword nofedges;
  /* Sammlung von Kanten, die von dem Contig abgehen */
  /* Speicherung der ID der Kanten statt Ptr auf Kanten? */
  struct GtScaffoldGraphEdge **edges;
  /* Zustand des Knotens */
  GraphState state;
};

/* Kante */
struct GtScaffoldGraphEdge{
  /* eindeutige ID fuer die Kante */
  GtUword id;
  /* Pointer zu dem Knoten, zu dem die Kante fuehrt */
  struct GtScaffoldGraphVertex *end;
  /* Pointer zu dem Knoten, von dem die Kante kommt */
  struct GtScaffoldGraphVertex *start;
  /* Abschaetzung der Entfernung der verbundenen Contigs */
  GtWord dist;
  /* Standardabweichung von der abgeschaetzten Entfernung */
  float stddev;
  /* Zustand der Kante */
  GraphState state;
  /* Anzahl der Distanzinformationen, die ein Anzeichen fuer die
  Verbindung der Contigs geben */
  GtUword numpairs;
  /* enthaelt die Richtung (Sense, Antisense) und welche
     Straenge die paired-Information enthalten (die gleiche
     Richtung oder das Reverse) */
  bool dir;
  bool comp;
};

/* Graph */
struct GtScaffoldGraph{
  struct GtScaffoldGraphVertex *vertices;
  GtUword nofvertices;
  struct GtScaffoldGraphEdge *edges;
  GtUword nofedges;
};

/* Datenstruktur fuer Queue-Element */
/* SD: Benötigt? */
typedef struct Pair{
  float dist;
  struct GtScaffoldGraphEdge edge;
} Pair;

/* Datenstruktur fuer Walk */
typedef struct Walk{
  GtUword nofedges;
  GtUword size;
  GtUword totalcontiglen;
  struct GtScaffoldGraphEdge *edges;
} Walk;

/* Datenstruktur fuer DistanceMap */
/* Datenstruktur fuer EdgeMap */


GtScaffoldGraph *new_graph(void){
  GtScaffoldGraph *graph;

  /* SD: malloc innerhalb des Konstruktors oder in der Caller Funktion? */
  graph = malloc(sizeof(*graph));
  graph->vertices = malloc(sizeof(*graph->vertices) * 5);
  graph->edges = malloc(sizeof(*graph->edges) * 6);
  graph->nofvertices = 5;
  graph->nofedges = 6;
  graph->vertices[0].id = 0;
  graph->vertices[0].edges = malloc(sizeof(*graph->edges) * 2);
  graph->vertices[0].edges[0] = &graph->edges[0];
  graph->vertices[0].edges[1] = &graph->edges[1];
  graph->vertices[0].edges[0]->id = 0;
  graph->vertices[0].edges[1]->id = 1;
  graph->vertices[0].edges[0]->dist = 2;
  graph->vertices[0].edges[1]->dist = 2;
  graph->vertices[0].nofedges = 2;
  graph->edges[0].start = &graph->vertices[0];
  graph->edges[1].start = &graph->vertices[0];

  graph->vertices[1].id = 1;
  graph->vertices[1].edges = malloc(sizeof(*graph->edges) * 3);
  graph->vertices[1].edges[0] = &graph->edges[0];
  graph->vertices[1].edges[1] = &graph->edges[3];
  graph->vertices[1].edges[2] = &graph->edges[4];
  graph->vertices[1].edges[0]->id = 0;
  graph->vertices[1].edges[1]->id = 1;
  graph->vertices[1].edges[2]->id = 2;
  graph->vertices[1].edges[0]->dist = 2;
  graph->vertices[1].edges[1]->dist = 2;
  graph->vertices[1].edges[2]->dist = 2;
  graph->vertices[1].nofedges = 3;
  graph->edges[0].start = &graph->vertices[1];
  graph->edges[3].start = &graph->vertices[1];
  graph->edges[4].start = &graph->vertices[1];

  graph->vertices[2].id = 2;
  graph->vertices[2].edges = malloc(sizeof(*graph->edges) * 2);
  graph->vertices[2].edges[0] = &graph->edges[1];
  graph->vertices[2].edges[1] = &graph->edges[2];
  graph->vertices[2].edges[0]->id = 0;
  graph->vertices[2].edges[1]->id = 1;
  graph->vertices[2].edges[0]->dist = 2;
  graph->vertices[2].edges[1]->dist = 2;
  graph->vertices[2].nofedges = 2;
  graph->edges[1].start = &graph->vertices[2];
  graph->edges[2].start = &graph->vertices[2];

  graph->vertices[3].id = 3;
  graph->vertices[3].edges = malloc(sizeof(*graph->edges) * 3);
  graph->vertices[3].edges[0] = &graph->edges[2];
  graph->vertices[3].edges[1] = &graph->edges[3];
  graph->vertices[3].edges[2] = &graph->edges[5];
  graph->vertices[3].edges[0]->id = 0;
  graph->vertices[3].edges[1]->id = 1;
  graph->vertices[3].edges[2]->id = 2;
  graph->vertices[3].edges[0]->dist = 2;
  graph->vertices[3].edges[1]->dist = 2;
  graph->vertices[3].edges[2]->dist = 2;
  graph->vertices[3].nofedges = 3;
  graph->edges[2].start = &graph->vertices[3];
  graph->edges[3].start = &graph->vertices[3];
  graph->edges[5].start = &graph->vertices[3];

  graph->vertices[4].id = 4;
  graph->vertices[4].edges = malloc(sizeof(*graph->edges) * 2);
  graph->vertices[4].edges[0] = &graph->edges[4];
  graph->vertices[4].edges[1] = &graph->edges[5];
  graph->vertices[4].edges[0]->id = 0;
  graph->vertices[4].edges[1]->id = 1;
  graph->vertices[4].edges[0]->dist = 2;
  graph->vertices[4].edges[1]->dist = 2;
  graph->vertices[4].nofedges = 2;
  graph->edges[4].start = &graph->vertices[4];
  graph->edges[5].start = &graph->vertices[4];

  graph->vertices[4].edges[0].end = &graph->vertices[3];
  graph->vertices[4].edges[1].end = &graph->vertices[4];
  graph->vertices[3].edges[0].end = &graph->vertices[1];
  graph->vertices[3].edges[1].end = &graph->vertices[3];
  graph->vertices[3].edges[2].end = &graph->vertices[3];
  graph->vertices[2].edges[0].end = &graph->vertices[3];
  graph->vertices[2].edges[1].end = &graph->vertices[2];
  graph->vertices[1].edges[0].end = &graph->vertices[0];
  graph->vertices[1].edges[1].end = &graph->vertices[1];
  graph->vertices[1].edges[2].end = &graph->vertices[4];
  graph->vertices[0].edges[0].end = &graph->vertices[2];
  graph->vertices[0].edges[1].end = &graph->vertices[0];

  return graph;
}

/* dotfile in datei filename ausgeben */
int write_graph(struct GtScaffoldGraph *g, char *filename) {
  int err = 0;

  FILE *f = fopen(filename, "w");
  if (f == NULL)
    return errno;
  
  /* TODO: errorhandling einfuehren */
  print_graph(g, f);

  fclose(f);

  return err;
}

/* dotfile ausgeben */
void print_graph(struct GtScaffoldGraph *g, FILE *f) {
  int i;
  struct GtScaffoldGraphVertex *v;
  struct GtScaffoldGraphEdge *e;
  char *color;

  /* die erste Zeile in der Dot-Datei schreiben */
  fprintf(f, "graph {\n");

  /* alle Knoten durchgehen und schreiben */
  for (i = 0; i < g->nofvertices; i++) {
    v = &g->vertices[i];

    if (v->state == GS_REPEAT || v->state == GS_POLYMORPHIC || v->state == GS_INCONSISTENT) {
      color = "gray";
    } else if (v->state == GS_VISITED) {
      color = "red";
    } else if (v->state == GS_PROCESSED) {
      color = "green";
    } else {
      color = "black";
    }

    fprintf(f, "%d [color=\"%s\"];\n", v->id, color);
  }

  /* alle Kanten durchgehen und schreiben */
  for (i = 0; i < g->nofedges; i++) {
    e = &g->edges[i];

    if (e->state == GS_REPEAT || e->state == GS_POLYMORPHIC || e->state == GS_INCONSISTENT) {
      color = "gray";
    } else if (e->state == GS_VISITED) {
      color = "red";
    } else if (e->state == GS_PROCESSED) {
      color = "green";
    } else {
      color = "black";
    }

    fprintf(f, "%d -- %d [color=\"%s\" label=\"%d\"];\n", e->start->id, e->end->id, color, e->dist);
  }

  /* die letzte Zeile schreiben */
  fprintf(f, "}\n");
}


/* SD: Noch nicht funktionstüchtig */
GtScaffoldGraph *gt_scaffolder_graph_new_from_file(const char *filename,
  int *had_err)
{
  FILE *infile;
  char buffer[BUFSIZ];
  GtScaffoldGraph *graph;

  graph = malloc(sizeof(*graph));
  infile = fopen(filename, "r");
  rewind(infile);
  while (fgets(buffer, BUFSIZ, infile) != NULL) {
    printf("%s\n",buffer);
  }
  fclose(infile);

  *had_err = 0;
  return graph;
}

/* Pruefung auf eindeutige Ordnung der Kanten edge1, edge 2 mit Wahrscheinlichkeit
   cutoff */
static bool gt_scaffolder_graph_ambiguousorder(const GtScaffoldGraphEdge edge1,    
      const GtScaffoldGraphEdge edge2, float cutoff){

  float expval, variance, interval, prob12, prob21;

  expval = edge1.dist - edge2.dist;
  /* sichere Multiplikation, Division */
  variance = 2 * pow(edge1.stddev,2) - pow(edge2.stddev,2);
  interval = -expval / sqrt(variance);
  /* Integralfunktion fehlt */
  prob12 = 0.5 * (1 + erf(interval) );
  prob21 = 1.0 - prob12;
  
  return (prob12 <= cutoff && prob21 <= cutoff);
}

/* Makierung polymorpher Kanten/Knoten und inkonsistenter Kanten im
   Scaffold Graphen */
int gt_scaffolder_graph_filtering(GtScaffoldGraph *graph, float pcutoff,
    float cncutoff, GtUword ocutoff){
  GtScaffoldGraphVertex vertex, polymorphic_vertex;
  GtScaffoldGraphEdge edge1, edge2;
  GtUword vid, eid_1, eid_2, eid_3, maxoverlap;
  float sum_copynum;
  unsigned int dir; /* int statt bool, weil Iteration bislang nicht möglich */
  int had_err = 0; 
  
  /* Iteration ueber alle Knoten */
  for (vid = 0; vid < graph->nofvertices; vid++){
    vertex = graph->vertices[vid];
    /* Iteration ueber alle Richtungen (Sense/Antisense) */
    for (dir = 0; dir < 2; dir++){ 
      /* Iteration ueber alle Kantenpaare */    
      for (eid_1 = 0; eid_1 < vertex.nofedges; eid_1++){
        for (eid_2 = eid_1+1; eid_2 < vertex.nofedges; eid_2++){
          edge1 = vertex.edges[eid_1];
          edge2 = vertex.edges[eid_2];
          if (edge1.dir == dir && edge2.dir == dir){
            /* Pruefung des Kantenpaares edge1, edge2 auf polymorphe Merkmale */  
            sum_copynum = edge1.end.copynum + edge2.end.copynum;       
            if (gt_scaffolder_graph_ambiguousorder(edge1, edge2, pcutoff) &&
                sum_copynum < cncutoff){
              /* Markierung Endknoten mit kleinerer estCopyNum als polymorph */
              if (edge1.end.copynum < edge2.end.copynum)
                polymorphic_vertex = edge1.end;
              else
                polymorphic_vertex = edge2.end;
              /* Markierung aller Sense- /Antisensekanten des polymorphen Knoten
                 als polymorph */   
              for (eid_3 = 0; eid_3 < polymorphic_vertex.nofedges; eid_3++)
                polymorphic_vertex.edges[eid_3].state = GS_POLYMORPHIC;
              polymorphic_vertex.state = GS_POLYMORPHIC;
            }
            /* SD: Nur das erste Paar polymoprh markieren? */
          }
        }
      }

      /* keine Pruefung auf inkonsistente Kanten fuer polymorphe Knoten
         notwendig */
      if (vertex.state == GS_POLYMORPHIC)
        break; 
      /* Iteration ueber alle nicht-polymorphen Kantenpaare in derselben Richtung */    
      for (eid_1 = 0; eid_1 < vertex.nofedges; eid_1++){
        for (eid_2 = eid_1+1; eid_2 < vertex.nofedges; eid_2++){
          edge1 = vertex.edges[eid_1];
          edge2 = vertex.edges[eid_2]; 
          if (edge1.dir == dir && edge2.dir == dir &&
              edge1.state != GS_POLYMORPHIC && edge2.state != GS_POLYMORPHIC){
            /* TODO: calculate overlapp*/               
          }
          maxoverlap = 10; /* Compiler meckert sonst über unbenutzte Variable */
        }
      }
      
     /* Pruefung aller Kantenpaare auf maximale Ueberlappung > ocutoff */
      if (maxoverlap > ocutoff){
        for (eid_1 = 0; eid_1 < vertex.nofedges; eid_1++)
         vertex.edges[eid_1].state = GS_INCONSISTENT;
      }
    }
  }
  /* SD: Knoten & Kanten werden nicht gelöscht, sondern der Einfachheit halber später geprüft*/
  return had_err;
}

/* Ueberpruefung ob Knoten terminal ist, d.h. nur sense oder antisense-Kanten
   vorliegen */
static bool gt_scaffolder_graph_isterminal(const GtScaffoldGraphVertex vertex){
  GtUword sense = 0, antisense = 0, eid;
  GtScaffoldGraphEdge edge;

  for (eid = 0; eid < vertex.nofedges; eid++){
    edge = vertex.edges[eid];
    if (edge.dir == SENSE)
      sense++;
    else
      antisense++;
  }

  return ((sense == 0 && antisense != 0) || (sense != 0 && antisense == 0));
}

/* Entfernung von Zyklen 
static void gt_scaffolder_removecycles(GtScaffoldGraph *graph){
}*/

/* Erstellung eines neuen Walks */ 
static Walk *gt_scaffolder_walk_new(void){
  Walk *walk;

  walk = malloc(sizeof(*walk));
  walk->totalcontiglen = 0;
  walk->size = 0;
  walk->nofedges = 0;
  return walk;
}

/* Loeschen eines Walks */
static void gt_scaffolder_walk_delete(Walk *walk){
  if (walk != NULL)
    free(walk->edges);
  free(walk);
}

/* Ausgabe der Contig-Gesamtlaenge eines Walks */
static GtUword gt_scaffolder_walk_getlength(Walk *walk){
  return walk->totalcontiglen;
}

/* Hinzufuegen einer Kante zum Walk */
static void gt_scaffolder_walk_addegde(Walk *walk, GtScaffoldGraphEdge edge){
  if (walk->size == walk->nofedges){
    walk->size += 10;
    walk->edges = realloc(walk->edges, walk->size*sizeof(*walk->edges));
  }
  //twin mit NULL initialisieren, wegen realloc 
  edge.twin = NULL;
  walk->edges[walk->nofedges] = edge;
  walk->totalcontiglen += edge.end.seqlen;
  walk->nofedges++;
}

/* Konstruktion des Scaffolds mit groesster Contig-Gesamtlaenge */
void gt_scaffolder_makescaffold(GtScaffoldGraph *graph){
  GtScaffoldGraphVertex vertex, currentvertex, nextvertex, nextendvertex,
                        endvertex;
  GtScaffoldGraphEdge edge, nextedge, reverseedge, *edgemap;
  GtUword vid, eid, ccnumber, lengthcwalk, lengthbestwalk;
  GtQueue *vqueue, *wqueue;
  float distance, *distancemap;
  Pair *pair, *updatepair;
  bool dir;
  Walk *bestwalk, *currentwalk;
  
  /* Entfernung von Zyklen 
     gt_scaffolder_removecycles(graph); */

  /* Iteration ueber alle Knoten, Makierung aller Knoten als unbesucht */
  for (vid = 0; vid < graph->nofvertices; vid++){
    vertex = graph->vertices[vid];
    /* SD: Existieren Repeat-Knoten, nach Prozessierung der AStatistik? */
    if (vertex.state == GS_REPEAT || vertex.state == GS_POLYMORPHIC)
      continue;
    vertex.state = GS_UNVISITED;
  }
  
 /* BFS-Traversierung durch Zusammenhangskomponenten des Graphen,
    siehe GraphSearchTree.h */
  ccnumber = 0;
  vqueue = gt_queue_new();
  pair = malloc(sizeof(*pair));
  updatepair = malloc(sizeof(*updatepair));
  distancemap = calloc(graph->nofvertices, sizeof(*distancemap));
  edgemap = malloc(sizeof(*edgemap)*graph->nofvertices);

  for (vid = 0; vid < graph->nofvertices; vid++){
    vertex = graph->vertices[vid];
    if (vertex.state == GS_REPEAT || vertex.state == GS_POLYMORPHIC ||
        vertex.state == GS_VISITED)
      continue;
    ccnumber += 1; 
    vertex.state = GS_PROCESSED;
    gt_queue_add(vqueue, &vertex);

    while (gt_queue_size(vqueue) != 0){
      currentvertex = *(GtScaffoldGraphVertex*)gt_queue_get(vqueue);
      //currentvertex.cc = ccnumber;

      /* BFS-Traversierung innerhalb aktueller Zusammenhangskomponente
         ausgehend von terminalen Knoten zu terminalen Knoten */
      lengthbestwalk = 0;
      bestwalk = gt_scaffolder_walk_new();
      wqueue = gt_queue_new(); 

      if (gt_scaffolder_graph_isterminal(vertex)){
        dir = vertex.edges[0].dir;
        for (eid = 0; eid < vertex.nofedges; eid++){
          edge = vertex.edges[eid];
          endvertex = edge.end;
          pair->edge = edge;
          pair->dist = edge.dist;
        
          distancemap[endvertex.id] = edge.dist;
          edgemap[endvertex.id] = edge;

          gt_queue_add(wqueue, pair);
        } 
        while(gt_queue_size(wqueue) != 0){
          pair = (Pair*)gt_queue_get(wqueue);
          edge = pair->edge;
          endvertex = edge.end;

          /* Ruecktraversierung durch EdgeMap wenn terminaler Knoten erreicht,
             Konstruktion des Walks  */
          if (gt_scaffolder_graph_isterminal(endvertex)){
            
            currentwalk = gt_scaffolder_walk_new();
            currentvertex = endvertex;
            while (currentvertex.id != vertex.id){
              reverseedge = edgemap[currentvertex.id];
             /* Start NICHT end */
              currentvertex = reverseedge.end;
             /* Speicherung des aktuellen Walks */
             /* Kante vorher duplizieren */  
              gt_scaffolder_walk_addegde(currentwalk, reverseedge);              
            }
            
            /* Ermittelung Contig-Gesamtlaenge des aktuellen Walks  */
            lengthcwalk = gt_scaffolder_walk_getlength(currentwalk);
            if ( lengthcwalk > lengthbestwalk){
              gt_scaffolder_walk_delete(bestwalk);
              bestwalk = currentwalk;
              lengthbestwalk = lengthcwalk;
            }
            else
              gt_scaffolder_walk_delete(currentwalk);
          }

          /* SD: Terminal Set implementieren, bestWalk über Rücktraversierung */
          for (eid = 0; eid < endvertex.nofedges; eid++){
            nextedge = endvertex.edges[eid];
            if (nextedge.dir == dir){
              nextendvertex = nextedge.end;
              distance = pair->dist + nextedge.dist;

              if (distancemap[nextendvertex.id] == 0 ||
                  distancemap[nextendvertex.id] > distance){
                distancemap[nextendvertex.id] = distance;
                edgemap[nextendvertex.id] = nextedge;
                updatepair->edge = nextedge;
                updatepair->dist = distance;
                gt_queue_add(wqueue, updatepair); 
              }
            }
          }
        }
      }


      currentvertex.state = GS_VISITED;
      for (eid = 0; eid < currentvertex.nofedges; eid++){
        edge = currentvertex.edges[eid];
        nextvertex = edge.end;
        if (vertex.state == GS_REPEAT || vertex.state == GS_POLYMORPHIC)
          continue;
        if (nextvertex.state == GS_UNVISITED){
          nextvertex.state = GS_PROCESSED;
          gt_queue_add(vqueue, &nextvertex);
        }
      }
    }
  }
}



