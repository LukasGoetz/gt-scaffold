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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/init_api.h"
#include "core/types_api.h"

#include "gt_scaffolder_graph.h"
#include "gt_scaffolder_algorithms.h"
#include "gt_scaffolder_parser.h"

/* adapted from SGA examples */
#define MIN_CONTIG_LEN 200

#define COPY_NUM_CUTOFF 0.3
#define ASTAT_NUM_CUTOFF 20.0

#define PROBABILITY_CUTOFF 0.01
#define COPY_NUM_CUTOFF_2 1.5
#define OVERLAP_CUTOFF 400

int main(int argc, char **argv)
{
  GtError *err;
  GtScaffolderGraph *graph;
  char mode[32], *contig_filename, *dist_filename, *astat_filename;
  int had_err = 0;

  if (sscanf(argv[1], "%s", mode) != 1) {
    fprintf(stderr,"Usage: %s <mode> <arguments>" ,argv[0]);
    exit(EXIT_FAILURE);
  }

  /* initialize */
  gt_lib_init();
  /* create error object */
  err = gt_error_new();

  if(strcmp(mode, "graph") == 0) {
    /* SK: err auch uebergeben */
    /* Create graph with wrapper construction function and delete it */
    gt_scaffolder_graph_test(5, 8, false, 0, false, 0, false);

    /* Create graph, only initialize vertices, delete it */
    gt_scaffolder_graph_test(5, 8, true, 0, false, 0, false);

    /* Create graph, initialize and create vertices, delete it */
    gt_scaffolder_graph_test(5, 8, true, 5, false, 0, false);

    /* Create graph, initialize vertices and edges, create vertices, delete it */
    gt_scaffolder_graph_test(5, 8, true, 5, true, 0, false);

    /* Create graph, initialize and create vertices and edges, delete it */
    gt_scaffolder_graph_test(5, 8, true, 5, true, 8, false);

    /* Create graph, initialize and create vertices and edges, print it,
       delete it */
    gt_scaffolder_graph_test(5, 8, true, 5, true, 8, true);
  }

  if (strcmp(mode, "parser") == 0) {
    if (argc != 3) {
      fprintf(stderr, "Usage: <DistEst file>\n");
    } else {
      dist_filename = argv[2];
      had_err = gt_scaffolder_parser_read_distances_test(dist_filename,
                "gt_scaffolder_parser_test_read_distances.de", err);
    }
  }

  if (strcmp(mode, "filter") == 0) {
    if (argc != 5) {
      fprintf(stderr, "Usage:<FASTA-file with contigs> <DistEst file> "
                      "<astat file>\n");
    } else {
      graph = NULL;
      contig_filename = argv[2];
      dist_filename = argv[3];
      astat_filename = argv[4];

      graph = gt_scaffolder_graph_new_from_file(contig_filename, MIN_CONTIG_LEN,
              dist_filename, err);

      /* load astatistics and copy number from file */
      had_err = gt_scaffolder_graph_mark_repeats(astat_filename, graph,
                COPY_NUM_CUTOFF, ASTAT_NUM_CUTOFF, err);

      if (had_err == 0) {
        gt_scaffolder_graph_print(graph,
              "gt_scaffolder_algorithms_test_filter_repeats.dot", err);
        /* mark polymorphic vertices, edges and inconsistent edges */
        had_err = gt_scaffolder_graph_filter(graph, PROBABILITY_CUTOFF,
                  COPY_NUM_CUTOFF_2, OVERLAP_CUTOFF);
        if (had_err == 0)
          gt_scaffolder_graph_print(graph,
              "gt_scaffolder_algorithms_test_filter_polymorphism.dot", err);
      }
      if (had_err != 0)
        fprintf(stderr,"ERROR: %s\n",gt_error_get(err));

      gt_scaffolder_graph_delete(graph);
    }
  }

  gt_error_delete(err);
  gt_lib_clean();
  return EXIT_SUCCESS;
}
