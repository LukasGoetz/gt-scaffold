/*
  Copyright (c) 2014 Dorle Osterode, Stefan Dang, Lukas Götz
  Copyright (c) 2014 Center for Bioinformatics, University of Hamburg

  Permission to use, copy, modify, and distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include "gt_scaffolder_graph.h"

#ifndef GT_SCAFFOLDER_ALGORITHMS_H
#define GT_SCAFFOLDER_ALGORITHMS_H

/* load astatics and copy number of every contig and mark repeated contigs */
int gt_scaffolder_graph_mark_repeats(const char *filename,
                                     GtScaffolderGraph *graph,
                                     float copy_num_cutoff,
                                     float astat_cutoff,
                                     GtError *err);

/* check if unique order of edges <*edge1>, <*edge2> with probability
   <cutoff> exists */
bool
gt_scaffolder_graph_ambiguousorder(const GtScaffolderGraphEdge *edge1,
                                   const GtScaffolderGraphEdge *edge2,
                                   float cutoff);

GtUword gt_scaffolder_calculate_overlap(GtScaffolderGraphEdge *edge1,
                                               GtScaffolderGraphEdge *edge2);

void
gt_scaffolder_graph_check_mark_polymorphic(GtScaffolderGraphEdge *edge1,
                                           GtScaffolderGraphEdge *edge2,
                                           float pcutoff,
                                           float cncutoff);

/* mark polymorphic edges/vertices and inconsistent edges in scaffold graph */
int gt_scaffolder_graph_filter(GtScaffolderGraph *graph,
                               float pcutoff,
                               float cncutoff,
                               GtUword ocutoff);

/* check if vertex holds just sense or antisense edges */
bool
gt_scaffolder_graph_isterminal(const GtScaffolderGraphVertex *vertex);

/* create new walk */
GtScaffolderGraphWalk *gt_scaffolder_walk_new(void);

/* remove walk <*walk> */
void gt_scaffolder_walk_delete(GtScaffolderGraphWalk *walk);

/* add edge <*edge> to walk <*walk> */
void gt_scaffolder_walk_addegde(GtScaffolderGraphWalk *walk,
                                       GtScaffolderGraphEdge *edge);

GtScaffolderGraphWalk *gt_scaffolder_create_walk(GtScaffolderGraph *graph,
                 GtScaffolderGraphVertex *start);

/* Konstruktion des Scaffolds mit groesster Contig-Gesamtlaenge */
void gt_scaffolder_makescaffold(GtScaffolderGraph *graph);





#endif