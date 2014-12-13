#!/usr/bin/env bash

# Copyright (c) 2014 Dorle Osterode, Stefan Dang, Lukas Götz
# Copyright (c) 2014 Center for Bioinformatics, University of Hamburg
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

set -e -x

TESTDATA=../testdata

./test.x graph

diff gt_scaffolder_graph_test.dot \
  $TESTDATA/gt_scaffolder_graph_test_expected.dot

./test.x parser $TESTDATA/primary-contigs.fa $TESTDATA/libPE.de

diff gt_scaffolder_parser_test_complete.dot \
  $TESTDATA/gt_scaffolder_parser_test_complete_expected.dot

./test.x filter $TESTDATA/primary-contigs.fa $TESTDATA/libPE.de $TESTDATA/libPE.astat

diff gt_scaffolder_algorithms_test_filter_repeats.dot \
  $TESTDATA/gt_scaffolder_algorithms_test_filter_repeats_expected.dot
