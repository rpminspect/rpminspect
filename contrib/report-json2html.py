#!/bin/env python3

import os
import sys
import json
from jinja2 import Template

JINJA_TEMPLATE = """
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">

    <!-- Bootstrap CSS -->
    <link rel="stylesheet" href="https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/css/bootstrap.min.css" integrity="sha384-ggOyR0iXCbMQv3Xipma34MD+dH/1fQ784/j6cY/iJTQUOhcWr7x9JvoRxT2MZw1T" crossorigin="anonymous">

    <title>rpminspect report</title>
</head>
<body>
    <div class="container">
      <div class="row">
        <h1>rpminspect report</h1>
      </div>
      <hr>
      <nav class="nav">
        {% for check_type in data %}
        <a class="nav-link" href="#{{ check_type }}">{{ check_type }}</a>
        {% endfor %}
      </nav>
      <hr>
      {% for check_type, data_list in data.items() %}
      <div class="row bg-dark text-white">
        <h2 id="{{ check_type }}">{{ check_type }}</h2>
      </div>
      <div class="row">
        {% for result_item in data_list %}
        <div class="col-12">
          <ul class="list-group list-group-flush">
            {% if result_item.result == 'OK' %}
            <li class="list-group-item bg-success text-white mb-2 mt-2 rounded rounded-sm">
            {% endif %}
            {% if result_item.result == 'INFO' %}
            <li class="list-group-item bg-info text-white mb-2 mt-2 rounded rounded-sm">
            {% endif %}
            {% if result_item.result == 'VERIFY' %}
            <li class="list-group-item bg-warning text-dark mb-2 mt-2 rounded rounded-sm">
            {% endif %}
            {% if result_item.result == 'BAD' %}
            <li class="list-group-item bg-danger text-dark mb-2 mt-2 rounded rounded-sm">
            {% endif %}
              <p class="text-right font-weight-bold">{{ result_item.result }}</p>
              {% if 'message' in result_item %}<p><span class="font-weight-bold">Message:</span> {{ result_item.message }}</p>{% endif %}
              {% if 'waiver authorization' in result_item %}<p><span class="font-weight-bold">Waiver:</span> {{ result_item['waiver authorization'] }}</p>{% endif %}
              {% if 'remedy' in result_item %}<p><span class="font-weight-bold">Remedy:</span> {{ result_item.remedy }}</p>{% endif %}
            </li>
          </ul>
        </div>
        {% endfor %}
      </div>
      {% endfor %}
    </div>
</body>
</html>
"""


def load_jinja_template():
    return Template(JINJA_TEMPLATE)


def load_json_report(path):
    with open(path) as fd:
        data = json.load(fd)
    return data


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Error: provide the rpminspect json report path")
        sys.exit(1)
    template = load_jinja_template()
    data = load_json_report(os.path.expanduser(sys.argv[1]))
    html_report = template.render(data=data)
    open("rpminspect.html", "w").write(html_report)
    print("Wrote rpminspect.html.")
