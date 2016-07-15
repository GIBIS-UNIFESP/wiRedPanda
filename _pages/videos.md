---
layout: inner
title: Videos
permalink: /videos/
lead_text: There are some projects developed by the Digital Circuits class at UNIFESP
---
{% for video in site.data.videos.links %}

<center>
<h2>{{ video.title }}</h2>
<iframe width="560" height="315" src="https://www.youtube.com/embed/{{ video.id }}" frameborder="0" allowfullscreen></iframe>
</center>
{% endfor %}
